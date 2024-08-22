#include <bits/stdc++.h>
#define PAGE_SIZE 4096
using namespace std;

// Make a struct for employee details
typedef struct {
    int empID;
    char name[20];
    int age;
    int salary;
    int deptID; // Department ID will be the common column
} Employee;

// Make a struct for department details
typedef struct {
    int deptID;
    char name[20];
    char location[20];
} Department;

int main() {
    FILE *fp,*fp2;
    fp = fopen("employee.bin", "wb");
    fp2 = fopen("employee2.bin","wb");
    Employee emp[1000];
    for(int i=0; i<1000; i++){
        emp[i].empID = i + 1;
        string name = "Employee" + to_string(i);
        strcpy(emp[i].name, name.c_str());
        emp[i].age = rand()%20+20;
        emp[i].salary = rand()%5000 + 30000;
        emp[i].deptID = rand()%5 + 1; // Assuming 5 departments
        cout << emp[i].empID << " " << emp[i].name << " " << emp[i].age << " " << emp[i].salary << " " << emp[i].deptID << endl;
    }

    int recordSizeEmp = sizeof(Employee);

    // Write employee data to file
    // Assuming PAGE_SIZE as the page size
    int i = 0;
    int totalRecordsEmp = 1000;
    while(1) {
        int numLeft = PAGE_SIZE;
        int possible = (PAGE_SIZE - 4) / recordSizeEmp;
        int recordsInPage = min(possible, totalRecordsEmp);
        fwrite(&recordsInPage, sizeof(int), 1, fp);
        fwrite(&recordsInPage, sizeof(int), 1, fp2);
        totalRecordsEmp -= recordsInPage;
        numLeft -= sizeof(int);

        while(numLeft >= recordSizeEmp) {
            fwrite(&emp[i], recordSizeEmp, 1, fp);
            fwrite(&emp[i], recordSizeEmp, 1, fp2);
            numLeft -= recordSizeEmp;
            i++;
            if(i == 1000) break;
        }
        while(numLeft > 0) {
            char c = '\0';
            fwrite(&c, sizeof(char), 1, fp);
            fwrite(&c, sizeof(char), 1, fp2);
            numLeft--;
        }
        if(i == 1000) break;
    }
    

    fclose(fp);

    fp = fopen("department.bin", "wb");

    Department dept[5]; // Assuming 5 departments
    for(int i = 0; i < 5; i++) {
        dept[i].deptID = i + 1;
        string name = "Department" + to_string(i);
        strcpy(dept[i].name, name.c_str());
        string location = "Location" + to_string(i);
        strcpy(dept[i].location, location.c_str());
        cout << dept[i].deptID << " " << dept[i].name << " " << dept[i].location << endl;
    }

    int recordSizeDept = sizeof(Department);

    // Write department data to file
    i = 0;
    while(1) {
        int numLeft = PAGE_SIZE;
        int possible = (PAGE_SIZE - 4) / recordSizeDept;
        int recordsInPage = min(possible, 5); // Number of departments
        fwrite(&recordsInPage, sizeof(int), 1, fp);
        numLeft -= sizeof(int);

        while(numLeft >= recordSizeDept) {
            fwrite(&dept[i], recordSizeDept, 1, fp);
            numLeft -= recordSizeDept;
            i++;
            if(i == 5) break;
        }
        while(numLeft > 0) {
            char c = '\0';
            fwrite(&c, sizeof(char), 1, fp);
            numLeft--;
        }
        if(i == 5) break;
    }

    fclose(fp);
}
