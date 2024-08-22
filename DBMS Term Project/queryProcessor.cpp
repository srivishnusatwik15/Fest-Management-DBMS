#include "queryProcessor.hpp"
#include <chrono>
// Constructor for QueryProcessor
QueryProcessor::QueryProcessor(int numFrames, int replacementPolicy) {
    if (replacementPolicy == CLOCK) bufferManager = new ClockBufferManager(numFrames);
    else if (replacementPolicy == LRU) bufferManager = new LRUBufferManager(numFrames);
    else if (replacementPolicy == MRU) bufferManager = new MRUBufferManager(numFrames);
    else if (replacementPolicy == FIFO) bufferManager = new FIFOBufferManager(numFrames);
    else {
        cout << "Invalid replacement policy\n";
        exit(1);
    }
}

// Process a select query
void QueryProcessor::processSelectQuery(FILE *fp, int col, string value) {
    int numPages = 0;
    int recordSize = sizeof(Employee);
    
    auto start = std::chrono::high_resolution_clock::now();
    // Get the number of pages
    fseek(fp, 0, SEEK_END);
    numPages = ftell(fp) / PAGE_SIZE;
    fseek(fp, 0, SEEK_SET);

    // Iterate over all pages
    for (int i = 0; i < numPages; ++i) {
        // Get page from buffer
        char *pageData = bufferManager->getPage(fp, i);
        int numRecords;
        memcpy(&numRecords, pageData, sizeof(int));
        pageData += sizeof(int);
        while (numRecords--) {
            Employee emp;
            memcpy(&emp, pageData, recordSize);
            pageData += recordSize;
            switch (col) {
                case 2:
                    if (strcmp(emp.name, value.c_str()) == 0)
                        cout << "Employee ID: " << emp.empID << "  Name: " << emp.name << "  Age: " << emp.age << "  Salary: " << emp.salary << "  Department ID: " << emp.deptID << endl;
                    break;
                case 3:
                    if (emp.age == stoi(value))
                        cout << "Employee ID: " << emp.empID << "  Name: " << emp.name << "  Age: " << emp.age << "  Salary: " << emp.salary << "  Department ID: " << emp.deptID << endl;
                    break;
                case 4:
                    if (emp.salary == stoi(value))
                        cout << "Employee ID: " << emp.empID << "  Name: " << emp.name << "  Age: " << emp.age << "  Salary: " << emp.salary << "  Department ID: " << emp.deptID << endl;
                    break;
                case 5:
                    if (emp.deptID == stoi(value))
                        cout << "Employee ID: " << emp.empID << "  Name: " << emp.name << "  Age: " << emp.age << "  Salary: " << emp.salary << "  Department ID: " << emp.deptID << endl;
                    break;
                case 1:
                    if (emp.empID == stoi(value))
                    cout << "Employee ID: " << emp.empID << "  Name: " << emp.name << "  Age: " << emp.age << "  Salary: " << emp.salary << "  Department ID: " << emp.deptID << endl;
                    break;
                default:
                    cout << "Invalid column number!" << endl;
                    return;
            }
        }
        // Unpin page
        bufferManager->unpinPage(fp, i);
    }
    auto end = std::chrono::high_resolution_clock::now(); // Stop measuring time
    std::chrono::duration<double> duration = end - start; // Calculate duration in seconds

    cout << "Time taken to process the select query: " << duration.count() << " seconds" << endl;
    cout << "Page Accesses: " << bufferManager->getStats().accesses << endl;
    cout << "Page fault and Disk Reads: " << bufferManager->getStats().diskreads << endl;
    cout << "Page Hits: " << bufferManager->getStats().pageHits << endl;
}

// Process a join query
void QueryProcessor::processJoinQuery(FILE *fp1, FILE *fp2, int col1, int col2) {
    int numPages1 = 0;
    int recordSize1 = sizeof(Employee);

    int numPages2 = 0;
    int recordSize2 = sizeof(Department);
    auto start = std::chrono::high_resolution_clock::now();
    // Get the number of pages for employee and department files
    fseek(fp1, 0, SEEK_END);
    numPages1 = ftell(fp1) / PAGE_SIZE;
    fseek(fp1, 0, SEEK_SET);

    fseek(fp2, 0, SEEK_END);
    numPages2 = ftell(fp2) / PAGE_SIZE;
    fseek(fp2, 0, SEEK_SET);

    for (int i = 0; i < numPages1; ++i) {
        char *pageData1 = bufferManager->getPage(fp1, i);
        if (pageData1 == NULL) {
            cout << "Number of Frames is too small\n";
            exit(0);
        }
        for (int j = 0; j < numPages2; ++j) {
            char *pageData2 = bufferManager->getPage(fp2, j);
            if (pageData2 == NULL) {
                cout << "Number of Frames is too small\n";
                exit(0);
            }
            int page1Offset = 0;
            int numRecords1;
            memcpy(&numRecords1, pageData1, sizeof(int));
            page1Offset += sizeof(int);
            while (numRecords1--) {
                Employee emp;
                memcpy(&emp, pageData1 + page1Offset, recordSize1);
                page1Offset += recordSize1;
                int numRecords2;
                memcpy(&numRecords2, pageData2, sizeof(int));
                int page2Offset = sizeof(int);
                while (numRecords2--) {
                    Department dept;
                    memcpy(&dept, pageData2 + page2Offset, recordSize2);
                    page2Offset += recordSize2;

                    if (col1 == 5 && col2 == 1) {
                        if (emp.deptID == dept.deptID)
                            cout << "Employee Name: " << emp.name << "  Employee Age: " << emp.age << "  Employee Salary: " << emp.salary << "  Department Name: " << dept.name << "  Department Location: " << dept.location << endl;
                    } else if (col1 == 1 && col2 == 5) {
                        if (emp.deptID == dept.deptID)
                            cout << "Department Name: " << dept.name << "  Department Location: " << dept.location << "  Employee Name: " << emp.name << "  Employee Age: " << emp.age << "  Employee Salary: " << emp.salary << endl;
                    }
                }
            }
            bufferManager->unpinPage(fp2, j);
        }
        bufferManager->unpinPage(fp1, i);
    }
    auto end = std::chrono::high_resolution_clock::now(); // Stop measuring time
    std::chrono::duration<double> duration = end - start; // Calculate duration in seconds

    cout << "Time taken to process the join query: " << duration.count() << " seconds" << endl;
    cout << "Page Accesses: " << bufferManager->getStats().accesses << endl;
    cout << "Page fault and Disk Reads: " << bufferManager->getStats().diskreads << endl;
    cout << "Page Hits: " << bufferManager->getStats().pageHits << endl;
}
void QueryProcessor::process_selfJoinQuery(FILE *fp1,FILE*fp2,int col1, int col2) {
    int numPages1 = 0;
    int recordSize1 = sizeof(Employee);

    int numPages2 = 0;
    int recordSize2 = sizeof(Employee);
    auto start = std::chrono::high_resolution_clock::now();
    // Get the number of pages for employee and department files
    fseek(fp1, 0, SEEK_END);
    numPages1 = ftell(fp1) / PAGE_SIZE;
    fseek(fp1, 0, SEEK_SET);
     fseek(fp2, 0, SEEK_END);
    numPages2 = ftell(fp2) / PAGE_SIZE;
    fseek(fp2, 0, SEEK_SET);
   
    for (int i = 0; i < numPages1; ++i) {
        char *pageData1 = bufferManager->getPage(fp1, i);
        if (pageData1 == NULL) {
            cout << "Number of Frames is too small\n";
            exit(0);
        }
        for (int j = 0; j < numPages2; ++j) {
            char *pageData2 = bufferManager->getPage(fp2, j);
            if (pageData2 == NULL) {
                cout << "Number of Frames is too small\n";
                exit(0);
            }
            int page1Offset = 0;
            int numRecords1;
            memcpy(&numRecords1, pageData1, sizeof(int));
            page1Offset += sizeof(int);
            while (numRecords1--) {
                Employee emp1;
                memcpy(&emp1, pageData1 + page1Offset, recordSize1);
                page1Offset += recordSize1;
                int numRecords2;
                memcpy(&numRecords2, pageData2, sizeof(int));
                int page2Offset = sizeof(int);
                while (numRecords2--) {
                    Employee emp2;
                    memcpy(&emp2, pageData2 + page2Offset, recordSize2);
                    page2Offset += recordSize2;

                    // Apply join condition
                if (col1 == 5  && col2 == 5) { // Join condition based on departmentID
                    if (emp1.deptID == emp2.deptID && emp1.empID != emp2.empID) {
                        cout << "Employee 1 ID: " << emp1.empID << ", Department ID: " << emp1.deptID << ", Name: " << emp1.name << endl;
                        cout << "Employee 2 ID: " << emp2.empID << ", Department ID: " << emp2.deptID << ", Name: " << emp2.name << endl;
                        cout<<"  "<<endl;
                         cout<<"  "<<endl;
                    }
                } // Add more conditions for other columns if needed
               else  if (col1 == 4  && col2==4) { // Join condition based on departmentID and salary
                    if ( emp1.salary == emp2.salary && emp1.empID != emp2.empID) {
                        cout << "Employee 1 ID: " << emp1.empID << ", Department ID: " << emp1.deptID << ", Salary: " << emp1.salary << ", Name: " << emp1.name << endl;
                        cout << "Employee 2 ID: " << emp2.empID << ", Department ID: " << emp2.deptID << ", Salary: " << emp2.salary << ", Name: " << emp2.name << endl;
                        cout<<"  "<<endl;
                         cout<<"  "<<endl;
                    }
                }
                else  if (col1 == 3 && col2==3 ) { // Join condition based on departmentID and salary
                    if ( emp1.age == emp2.age && emp1.empID != emp2.empID) {
                        cout << "Employee 1 ID: " << emp1.empID << ", Department ID: " << emp1.deptID << ", Age: " << emp1.age << ", Name: " << emp1.name << endl;
                        cout << "Employee 2 ID: " << emp2.empID << ", Department ID: " << emp2.deptID << ", Age: " << emp2.age << ", Name: " << emp2.name << endl;
                        cout<<"  "<<endl;
                         cout<<"  "<<endl;
                    }
                }
                }
            }
            bufferManager->unpinPage(fp2, j);
        }
        bufferManager->unpinPage(fp1, i);
    }
    auto end = std::chrono::high_resolution_clock::now(); // Stop measuring time
    std::chrono::duration<double> duration = end - start; // Calculate duration in seconds

    cout << "Time taken to process the join query: " << duration.count() << " seconds" << endl;
    cout << "Page Accesses: " << bufferManager->getStats().accesses << endl;
    cout << "Page fault and Disk Reads: " << bufferManager->getStats().diskreads << endl;
    cout << "Page Hits: " << bufferManager->getStats().pageHits << endl;
}


int main() {
    FILE *fp1 = fopen("employee.bin", "rb");
    FILE *fp3 = fopen("employee2.bin", "rb");
    FILE *fp2 = fopen("department.bin", "rb");

    if (!fp1 || !fp2) {
        cerr << "Error opening files!" << endl;
        return 1;
    }

    int numFrames;
    cout << "Enter the number of frames: ";
    cin >> numFrames;
    cout<<"Select a Replacement Algorithm:\n";
    cout<<"1 LRU, 2 MRU, 3 CLOCK,4 FIFO: ";
    int choice;
    cin>>choice;

    QueryProcessor qp(numFrames,choice);

    int queryType, col;
    string value;
    cout << "Enter 1 for select query | 2 for join query 3 for self join query: ";
    cin >> queryType;

    if (queryType == 1) {
        cout << "Enter column number (1:Employee ID 2: Name, 3: Age, 4: Salary, 5: Department ID): ";
        cin >> col;
        cout << "Enter value to get all matching records: ";
        cin >> value;
        qp.processSelectQuery(fp1, col, value);
    }
     else if (queryType == 2) {
        int col1, col2;
        cout << "Enter column number of Database 1 (1:Employee ID 2: Name, 3: Age, 4: Salary, 5: Department ID): ";
        cin >> col1;
        cout << "Enter column number of Database 2 (1:Department ID 2: Department Name, 3: Department Location): ";
        cin >> col2;
        qp.processJoinQuery(fp1, fp2, col1, col2);
    }
    else if(queryType == 3){
        int col;
        cout << "Enter column number1 of Database 1 (1:Employee id 2: Name, 3: Age, 4: Salary, 5: Department ID): ";
        cin >> col;
        qp.process_selfJoinQuery(fp1,fp3,col, col);
    }
    
    else {
        cout << "Invalid query type!" << endl;
        return 1;
    }

    fclose(fp1);
    fclose(fp2);
    return 0;
}
