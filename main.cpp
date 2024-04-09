#include <istream>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <iomanip>

using namespace std;

const long long SECONDS_PER_DAY = 3600L * 24L;
/////function related to .csv files
enum class CSVState
{
    UnquotedField,
    QuotedField,
    QuotedQuote
};

vector<string> readCSVRow(const string &row)
{
    CSVState state = CSVState::UnquotedField;
    vector<string> fields{""};
    size_t i = 0; // index of the current field
    for (char c : row)
    {
        switch (state)
        {
        case CSVState::UnquotedField:
            switch (c)
            {
            case ',': // end of field
                fields.push_back("");
                i++;
                break;
            case '"':
                state = CSVState::QuotedField;
                break;
            default:
                fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedField:
            switch (c)
            {
            case '"':
                state = CSVState::QuotedQuote;
                break;
            default:
                fields[i].push_back(c);
                break;
            }
            break;
        case CSVState::QuotedQuote:
            switch (c)
            {
            case ',': // , after closing quote
                fields.push_back("");
                i++;
                state = CSVState::UnquotedField;
                break;
            case '"': // "" -> "
                fields[i].push_back('"');
                state = CSVState::QuotedField;
                break;
            default: // end of quote
                state = CSVState::UnquotedField;
                break;
            }
            break;
        }
    }
    return fields;
}
/// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
vector<vector<string>> readCSV(istream &in)
{
    vector<vector<string>> table;
    string row;
    while (!in.eof())
    {
        getline(in, row);
        if (in.bad() || in.fail())
        {
            break;
        }
        auto fields = readCSVRow(row);
        table.push_back(fields);
    }
    return table;
}

vector<vector<string>> readCSVFromFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return {};
    }
    return readCSV(file);
}

void displayCSVData(const vector<vector<string>> &data)
{
    for (const auto &row : data)
    {
        for (const auto &field : row)
        {
            cout << field << "\t\t";
        }
        cout << endl;
    }
}

tm parseDate(const string &dateStr)
{
    tm date = {};
    istringstream ss(dateStr);
    ss >> get_time(&date, "%d/%m/%Y");
    return date;
}

string getTodaysDate()
{
    // Get the current time
    auto currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());

    // Convert to tm structure
    tm *dateInfo = localtime(&currentTime);

    // Format the date as "dd/mm/yyyy"
    ostringstream oss;
    oss << setw(2) << setfill('0') << dateInfo->tm_mday << "/"
        << setw(2) << setfill('0') << (dateInfo->tm_mon + 1) << "/"
        << (dateInfo->tm_year + 1900); // Year is years since 1900

    return oss.str();
}

int daysBetweenDates(const string &startDateStr, const string &endDateStr)
{
    tm startDate = parseDate(startDateStr);
    tm endDate = parseDate(endDateStr);

    // Convert tm to chrono::system_clock::time_point
    chrono::system_clock::time_point startDateTime = chrono::system_clock::from_time_t(mktime(&startDate));
    chrono::system_clock::time_point endDateTime = chrono::system_clock::from_time_t(mktime(&endDate));

    // Calculate the duration between the two dates
    chrono::duration<int> duration = chrono::duration_cast<chrono::duration<int>>(endDateTime - startDateTime);

    // Return the number of days
    return duration.count() / (24 * 60 * 60);
}

void displayCSVFile(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {
        cout << line << endl;
    }

    file.close();
}

void appendStringToCSV(const string &filename, const string &newString)
{
    ofstream file(filename, ios::app); // Open the file in append mode

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    file << newString << endl; // Append the new string to the next line

    file.close();
}

void deleteRowFromCSV(const string &filename, const string &firstEntryToDelete)
{
    ifstream inFile(filename);
    if (!inFile.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    vector<vector<string>> rowsToDelete;

    // Read the CSV content, excluding rows with the specified first entry
    string row;
    while (getline(inFile, row))
    {
        vector<string> fields = readCSVRow(row);
        if (!fields.empty() && fields[0] != firstEntryToDelete)
        {
            rowsToDelete.push_back(fields);
        }
    }

    inFile.close();

    // Write the updated content back to the CSV file
    ofstream outFile(filename);
    if (!outFile.is_open())
    {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }

    for (const auto &rowFields : rowsToDelete)
    {
        for (const auto &field : rowFields)
        {
            outFile << field << ",";
        }
        outFile << endl;
    }

    outFile.close();
}

bool doesRowExistInCSV(const string &filename, const string &firstEntryToCheck)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return false;
    }

    // Read the CSV content and check if the first entry exists in any row
    string row;
    while (getline(file, row))
    {
        vector<string> fields = readCSVRow(row);
        if (!fields.empty() && fields[0] == firstEntryToCheck)
        {
            file.close();
            return true; // Row with the given first entry exists
        }
    }

    file.close();
    return false; // Row with the given first entry does not exist
}

int getRowIndexInCSV(const string &filename, const string &firstEntryToFind)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return -1; // Return -1 to indicate an error
    }

    int rowIndex = 0; // Initialize index to 0
    string row;
    while (getline(file, row))
    {
        vector<string> fields = readCSVRow(row);
        if (!fields.empty() && fields[0] == firstEntryToFind)
        {
            file.close();
            return rowIndex; // Return the index of the row
        }
        rowIndex++;
    }

    file.close();
    return -1; // Return -1 if the row with the given first entry is not found
}

void writeCSVToFile(const string &filename, const vector<vector<string>> &data)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    for (const auto &row : data)
    {
        for (const auto &field : row)
        {
            file << field << ",";
        }
        file << endl;
    }

    file.close();
}

string getFirstElementOfLastRowInCSV(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return ""; // Return an empty string to indicate an error
    }

    string lastRow;
    string row;
    while (getline(file, row))
    {
        lastRow = row; // Update lastRow with the current row
    }

    file.close();

    // Extract the first element of the last row
    istringstream iss(lastRow);
    string firstElement;
    getline(iss, firstElement, ',');

    return firstElement;
}

string getRowByFirstEntryInCSV(const string &filename, const string &firstEntryToFind)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return ""; // Return an empty string to indicate an error
    }

    string foundRow;
    string row;
    while (getline(file, row))
    {
        vector<string> fields = readCSVRow(row);
        if (!fields.empty() && fields[0] == firstEntryToFind)
        {
            foundRow = row; // Store the entire row
            break;
        }
    }

    file.close();

    return foundRow;
}

vector<string> getIthElementsByFifthElementInCSV(const string &filename, const string &fifthElementToFind, size_t i)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return {}; // Return an empty vector to indicate an error
    }

    vector<string> ithElements;
    string row;
    while (getline(file, row))
    {
        vector<string> fields = readCSVRow(row);
        if (fields.size() > 4 && fields[4] == fifthElementToFind)
        {
            if (fields.size() > i)
            {
                ithElements.push_back(fields[i]); // Store the i-th element
            }
        }
    }

    file.close();

    return ithElements;
}

string getElementFromCSV(const string &filename, size_t rowIndex, size_t columnIndex)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return ""; // Return an empty string to indicate an error
    }

    string element;
    string row;
    size_t currentRowIndex = 0;

    while (getline(file, row))
    {
        if (currentRowIndex == rowIndex)
        {
            vector<string> fields = readCSVRow(row);
            if (columnIndex < fields.size())
            {
                element = fields[columnIndex]; // Store the element
                break;
            }
        }
        ++currentRowIndex;
    }

    file.close();

    return element;
}

vector<string> reduceBy15Percent(const vector<string> &inputVector)
{
    vector<string> resultVector;

    for (const string &str : inputVector)
    {
        // Convert string to double
        istringstream iss(str);
        double value;
        iss >> value;

        // Reduce value by 15%
        value *= 0.85;

        // Convert back to string with 2 decimal places
        ostringstream oss;
        oss << fixed << setprecision(2) << value;
        resultVector.push_back(oss.str());
    }

    return resultVector;
}

long long days_since(int year, int month, int day)
{
    // Current time since epoch
    time_t now = time(nullptr);

    // Convert year, month and day to a tm object
    tm beg = *localtime(&now);
    beg.tm_year = year - 1900;
    beg.tm_mon = month - 1;
    beg.tm_mday = day;

    // difftime returns seconds
    time_t bd = mktime(&beg);
    return static_cast<long long>(difftime(now, bd)) / SECONDS_PER_DAY;
}

bool isNumber(const string &str)
{
    // Check for empty string
    if (str.empty())
    {
        return false;
    }

    // Check for negative sign at the beginning
    size_t start = 0;
    if (str[0] == '-')
    {
        start = 1;
    }

    // Check each character in the string
    for (size_t i = start; i < str.length(); ++i)
    {
        if (!isdigit(str[i]) && str[i] != '.')
        {
            return false;
        }
    }

    return true;
}

void displayCSVTable(const string &filename, char delimiter = ',')
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    vector<vector<string>> data;

    // Read CSV data into a vector of vectors
    while (getline(file, line))
    {
        vector<string> row;
        istringstream ss(line);
        string cell;

        while (getline(ss, cell, delimiter))
        {
            row.push_back(cell);
        }

        data.push_back(row);
    }

    // Display CSV data in a table format
    for (const auto &row : data)
    {
        for (const auto &cell : row)
        {
            cout << setw(15) << left << cell;
        }
        cout << endl;
    }

    file.close();
}

bool valid4DigitID(string &ID)
{
    int i = ID.length();
    if (isNumber(ID) && i == 4 && !doesRowExistInCSV("customer.csv", ID))
    {
        return true;
    }
    return false;
}

bool valid4DigitID2(string &ID)
{
    int i = ID.length();
    if (isNumber(ID) && i == 4 && doesRowExistInCSV("customer.csv", ID))
    {
        return true;
    }
    return false;
}

bool valid3DigitID(string &ID)
{
    int i = ID.length();
    if (isNumber(ID) && i == 3 && !doesRowExistInCSV("employee.csv", ID))
    {
        return true;
    }
    return false;
}

bool valid3DigitID2(string &ID)
{
    int i = ID.length();
    if (isNumber(ID) && i == 3 && doesRowExistInCSV("employee.csv", ID))
    {
        return true;
    }
    return false;
}

bool valid4DigitCarID(string &ID)
{
    int i = ID.length();
    if (isNumber(ID) && i == 4 && doesRowExistInCSV("car.csv", ID))
    {
        return true;
    }
    return false;
}

void displayTable(const std::vector<std::vector<std::string>>& data) {
    // Determine the maximum width of each column
    std::vector<size_t> colWidths(data[0].size(), 0);
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            colWidths[i] = std::max(colWidths[i], row[i].size());
        }
    }

    // Display the data in a tabular format
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << std::setw(colWidths[i]) << std::left << row[i] << " ";
        }
        std::cout << std::endl;
    }
}
//////////////////////////////////////////////////////

/// classes /////////

class customer
{
public:
    string ID;
    string rentedCars;
    string fineDue;
    string customerRecords;
    string other_details;

    void displayData()
    {
        vector<vector<string>> csvData = readCSVFromFile("customer.csv");
        cout << "customerID\tRented cars\tFine due\tC records\tOther details" << endl;
        displayCSVData(csvData);
    }

    void addCustomer(const string &newString)
    {

        appendStringToCSV("customer.csv", newString);
    }

    void deleteCustomer(string &ID)
    {
        if (doesRowExistInCSV("customer.csv", ID))
        {
            deleteRowFromCSV("customer.csv", ID);
        }
        else
        {
            cout << "Customer with ID '" << ID << "' does not exist." << endl;
        }
    }

    void updateCustomerDetails(string &ID)
    {
        if (doesRowExistInCSV("customer.csv", ID))
        {
            int rowIndex, columIndex;
            cout << "What would you like to change:" << endl
                // << "1)Number of cars rented" << endl
                 << "1)Fine due" << endl
                 << "2)Customer records" << endl
                 << "3)Other details" << endl;
            cin >> columIndex;
            columIndex++;
            rowIndex = getRowIndexInCSV("customer.csv", ID);
            // cout << rowIndex << "," << columIndex;

            vector<vector<string>> csvData = readCSVFromFile("customer.csv");
            string temp;
            cout<<endl<<"Enter Enter new value:"<<endl;
            cin >> temp;
            csvData[rowIndex][columIndex] = temp;
            writeCSVToFile("customer.csv", csvData);
        }
        else
        {
            cout << "Customer with ID '" << ID << "' does not exist." << endl;
        }
    }

    string viewCustomerDetails(const string &customerEntryToFind)
    {
        return getRowByFirstEntryInCSV("customer.csv", customerEntryToFind);
    }


};

class employee
{
public:
    string ID;
    string rentedCars;
    string fineDue;
    string employeeRecords;
    string other_details;

    void displayData()
    {
        vector<vector<string>> csvData = readCSVFromFile("employee.csv");
        cout << "customerID\tRented cars\tFine due\tE records\tOther details" << endl;
        displayCSVData(csvData);
        // cout<<endl<<"Employee record is a number between 1 to 5, 5 beeing on the better half."<<endl;
    }
    void addEmployee(const string &newString)
    {

        appendStringToCSV("employee.csv", newString);
    }
    void deleteEmployee(string &ID)
    {
        if (doesRowExistInCSV("employee.csv", ID))
        {
            deleteRowFromCSV("employee.csv", ID);
        }
        else
        {
            cout << "employee with ID '" << ID << "' does not exist." << endl;
        }
    }
    void updateEmployeeDetails(string &ID)
    {
        if (doesRowExistInCSV("employee.csv", ID))
        {
            int rowIndex, columIndex;
            cout << "What would you like to change:" << endl
                 //<< "1)Number of cars rented" << endl
                 << "1)Fine due" << endl
                 << "2)employee records" << endl
                 << "3)Other details" << endl;
            cin >> columIndex;
            columIndex++;
            rowIndex = getRowIndexInCSV("employee.csv", ID);
            // cout << rowIndex << "," << columIndex;

            vector<vector<string>> csvData = readCSVFromFile("employee.csv");
            string temp;
            cout<<"Enter new details";
            cin >> temp;
            csvData[rowIndex][columIndex] = temp;
            writeCSVToFile("employee.csv", csvData);
        }
        else
        {
            cout << "Employee with ID '" << ID << "' does not exist." << endl;
        }
    }
    string viewEmployeeDetails(const string &EntryToFind)
    {
        return getRowByFirstEntryInCSV("employee.csv", EntryToFind);
    }
};

class car
{
public:
    string ID;
    string model;
    string condition;
    string other_details;
    string status;
    string price;
    string dueDate;

    void displayData()
    {
        vector<vector<string>> csvData = readCSVFromFile("car.csv");
        cout << "Car ID\t\tCar name\tCar condition\tOther Details\tCurrent renter(ID)\tcost(per day)\tDate of return" << endl;
        displayCSVTable("car.csv");
    }

    void addcar(const string &newString)
    {

        appendStringToCSV("car.csv", newString);
    }

    void deleteCar(string &ID)
    {
        if (doesRowExistInCSV("car.csv", ID))
        {
            deleteRowFromCSV("car.csv", ID);
        }
        else
        {
            cout << "Car with ID '" << ID << "' does not exist." << endl;
        }
    }

    void updateCarDetails(string &ID)
    {
        if (doesRowExistInCSV("car.csv", ID))
        {
            int rowIndex, columIndex;
            cout << "What would you like to change:" << endl
                 << "1)Condition of Car" << endl
                 << "2)Other detaitls" << endl
                 << "3)Price" << endl;
            cin >> columIndex;
            columIndex++;
            if (columIndex == 4)
            {
                columIndex++;
            }

            rowIndex = getRowIndexInCSV("car.csv", ID);
            //cout << rowIndex << "," << columIndex;

            vector<vector<string>> csvData = readCSVFromFile("car.csv");
            string temp;
            cout<<"Enter new value";
            cin >> temp;
            csvData[rowIndex][columIndex] = temp;
            writeCSVToFile("car.csv", csvData);
        }
        else
        {
            cout << "Car with ID '" << ID << "' does not exist." << endl;
        }
    }

    void showCarsByOwners(const string &owner)
    {
        vector<string> elements;
        elements = getIthElementsByFifthElementInCSV("car.csv", owner, 1);
        for (const auto &element : elements)
        {
            cout << element << "\t";
        }
    }

    void showAvaCarDetalils(int i)
    {
        vector<string> carID;
        vector<string> carName;
        vector<string> carPrice, temp;
        carID = getIthElementsByFifthElementInCSV("car.csv", "unrented", 0);
        carName = getIthElementsByFifthElementInCSV("car.csv", "unrented", 1);
        carPrice = getIthElementsByFifthElementInCSV("car.csv", "unrented", 5);
        if (i)
        {
            temp = reduceBy15Percent(carPrice);
            for (size_t i = 0; i < carID.size(); ++i)
            {
                cout << carID[i] << "\t" << temp[i] << "\t" << carName[i] << endl;
            }
        }
        else
        {
            for (size_t i = 0; i < carID.size(); ++i)
            {
                cout << carID[i] << "\t" << carPrice[i] << "\t" << carName[i] << endl;
            }
        }
    }

    void sentRequestC(string &customerID, string &carID)
    {
        string rowIndexS;
        int rowIndex, temp;
        vector<vector<string>> csvData = readCSVFromFile("car.csv");
        rowIndex = getRowIndexInCSV("customer.csv", customerID);
        customer c;
        c.customerRecords = getElementFromCSV("customer.csv", rowIndex, 3);
        c.rentedCars = getElementFromCSV("customer.csv", rowIndex, 1);
        if (stoi(c.customerRecords) <= stoi(c.rentedCars))
        {
            cout << "Your record is not good enough to rent more cars";
        }
        else
        {
            temp = stoi(c.rentedCars);
            temp++;
            c.rentedCars = to_string(temp);
            rowIndex = getRowIndexInCSV("car.csv", carID);
            string rowIndexS = customerID;
            csvData[rowIndex][4] = rowIndexS;
            writeCSVToFile("car.csv", csvData);
        }
    }

    void sentRequestE(string &customerID, string &carID)
    {
        string rowIndexS;
        int rowIndex, temp;
        vector<vector<string>> csvData = readCSVFromFile("car.csv");
        rowIndex = getRowIndexInCSV("employee.csv", customerID);
        employee c;
        c.employeeRecords = getElementFromCSV("employee.csv", rowIndex, 3);
        c.rentedCars = getElementFromCSV("employee.csv", rowIndex, 1);
        if (stoi(c.employeeRecords) <= stoi(c.rentedCars))
        {
            cout << "Your record is not good enough to rent more cars";
        }
        else
        {
            temp = stoi(c.rentedCars);
            temp++;
            c.rentedCars = to_string(temp);
            rowIndex = getRowIndexInCSV("car.csv", carID);
            string rowIndexS = customerID;
            csvData[rowIndex][4] = rowIndexS;
            writeCSVToFile("car.csv", csvData);
        }
    }

    void showDueDate(string &ID)
    {
        vector<string> carName;
        vector<string> carDueDate;
        carName = getIthElementsByFifthElementInCSV("car.csv", ID, 1);
        carDueDate = getIthElementsByFifthElementInCSV("car.csv", ID, 6);

        for (size_t i = 0; i < carDueDate.size(); ++i)
        {
            cout << carDueDate[i] << "\t" << carName[i] << endl;
        }
    }

    void returnCarC(string &carID)
    {
        vector<vector<string>> csvData = readCSVFromFile("car.csv");
        int rowIndex = getRowIndexInCSV("car.csv", carID);
        string userID = getElementFromCSV("car.csv", rowIndex, 4);
        int length = userID.length();

        if (length == 4)
        {
            string date = getElementFromCSV("car.csv", rowIndex, 6);
            int delay = daysBetweenDates(date, getTodaysDate());
            cout << delay;
            if (delay < 0)
            {
                delay = 0;
            }
            delay = delay * 10000;
            csvData[rowIndex][4] = "unrented";
            csvData[rowIndex][6] = "DD/MM/YYYY";
            writeCSVToFile("car.csv", csvData);
            vector<vector<string>> csvDataCustomer = readCSVFromFile("customer.csv");
            rowIndex = getRowIndexInCSV("customer.csv", userID);
            int userCars = stoi(getElementFromCSV("customer.csv", rowIndex, 1));
            int userFine = stoi(getElementFromCSV("customer.csv", rowIndex, 2));
            userCars--;
            userFine = userFine + delay;
            csvDataCustomer[rowIndex][1] = to_string(userCars);
            csvDataCustomer[rowIndex][2] = to_string(userFine);
            writeCSVToFile("customer.csv", csvDataCustomer);
        }
        else
        {
            string date = getElementFromCSV("car.csv", rowIndex, 6);
            int delay = daysBetweenDates(date, getTodaysDate());
            cout << delay;
            if (delay < 0)
            {
                delay = 0;
            }
            delay = delay * 10000;
            csvData[rowIndex][4] = "unrented";
            csvData[rowIndex][6] = "DD/MM/YYYY";
            writeCSVToFile("car.csv", csvData);
            vector<vector<string>> csvDataCustomer = readCSVFromFile("employee.csv");
            rowIndex = getRowIndexInCSV("employee.csv", userID);
            int userCars = stoi(getElementFromCSV("employee.csv", rowIndex, 1));
            int userFine = stoi(getElementFromCSV("employee.csv", rowIndex, 2));
            userCars--;
            userFine = userFine + delay;
            csvDataCustomer[rowIndex][1] = to_string(userCars);
            csvDataCustomer[rowIndex][2] = to_string(userFine);
            writeCSVToFile("employee.csv", csvDataCustomer);
        }
    }
};

///////////////////////////////////////////////
void iAmManager()
{
    customer c;
    employee e;
    car car;
    vector<vector<string>> csv2Data = readCSVFromFile("car.csv");
    int action, incustomer, inemployee, incar, tempInt, flag, flag2;
    string temp, carID;
    cout << "Chose action:" << endl
         << "1)Open customer details" << endl
         << "2)Open employee details" << endl
         << "3)Open car details" << endl
         << "4)Mark a car as return" << endl
        // << "5)Exit"<<endl
        ;
    while (1)
    {
        cin >> action;
        if (action < 1 || action > 4)
        {
            cout << endl
                 << "Invalid input, please try again" << endl;
            continue;
        }
        break;
    }

    switch (action)
    {
    case 1:
        c.displayData();
        cout << endl
             << "what would you like to do:" << endl
             << "1)Add customer" << endl
             << "2)Remove customer" << endl
             << "3)Update customer details" << endl
            //  << "4)Go back" << endl
            ;
        while (1)
        {
            cin >> incustomer;
            if (action < 1 || action > 3)
            {
                cout << endl
                     << "Invalid input, please try again" << endl;
                continue;
            }
            break;
        }
        switch (incustomer)
        {
        case 1:
            // while (1)
            // {
            cout << "Enter customer ID(4 digit):" << endl;
            while (1)
            {
                cin >> c.ID;
                if (!valid4DigitID(c.ID))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            // if (!valid4DigitID(c.ID))
            // {
            // 	cout<<"Invalid input, please try again"<<endl;
            // 	continue;
            // }
            // while (1)
            // {
            cout << "Enter number of cars he/She is renting:" << endl;
            cin >> c.rentedCars;
            // if (stoi(c.rentedCars) < 0 || stoi(c.rentedCars) > 5)
            // {
            // 	cout<<"Rented cars should from 0 to 5"<<endl;
            // 	continue;
            // }
            // break;
            // }
            c.fineDue = "0";
            c.customerRecords = "5";
            c.other_details = "Empty";
            temp = c.ID + "," + c.rentedCars + "," + c.fineDue + "," + c.customerRecords + "," + c.other_details;
            // cout << temp;
            c.addCustomer(temp);
            cout << endl
                 << "Customer Added" << endl;
            break;
        case 2:
            // while (1)
            // 	{
            cout << "Enter ID number of the customer you want to delete:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid4DigitID2(temp))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            // if (!valid4DigitID(temp))
            // {
            // 	cout<<"Invalid input, please try again"<<endl;
            // 	continue;
            // }
            //}
            c.deleteCustomer(temp);
            cout << endl
                 << "Customer deleated" << endl;
            break;
        case 3:
            // while (1)
            // {
            cout << "Enter ID number of the customer you want to update:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid4DigitID2(temp))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            // if (!valid4DigitID(temp))
            // {
            // 	cout<<"Invalid input, please try again"<<endl;
            // 	continue;
            // }
            //}
            c.updateCustomerDetails(temp);
            cout << "Update done";
            break;
        case 4:
            flag2 = 1;
            break;
            ;
        default:
            break;
            ;
        }
        break;
    case 2:
        e.displayData();
        cout << endl
             << "what would you like to do:" << endl
             << "1)Add employee" << endl
             << "2)Remove employee" << endl
             << "3)Update employee details" << endl;
        while (1)
        {
            cin >> inemployee;
            if (action < 1 || action > 3)
            {
                cout << endl
                     << "Invalid input, please try again" << endl;
                continue;
            }
            break;
        }
        switch (inemployee)
        {
        case 1:
            cout << "Enter employee ID(3 digit):" << endl;
            while (1)
            {
                cin >> e.ID;
                if (!valid3DigitID(c.ID))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            e.rentedCars = "0";
            e.fineDue = "0";
            e.employeeRecords = "5";
            e.other_details = "Empty";
            temp = e.ID + "," + e.rentedCars + "," + e.fineDue + "," + e.employeeRecords + "," + e.other_details;
            // cout << temp;
            cout << "Employee added";
            e.addEmployee(temp);
            break;
        case 2:
            cout << "Enter ID number of the employee you want to remove:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid3DigitID2(c.ID))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            e.deleteEmployee(temp);
            break;
        case 3:
            cout << "Enter ID number of the employee you want to update:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid3DigitID2(c.ID))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            e.updateEmployeeDetails(temp);
            break;
        default:
            break;
        }
        break;
    case 3:
        displayTable(csv2Data);
        cout << endl
             << "what would you like to do:" << endl
             << "1)Add car" << endl
             << "2)Remove car" << endl
             << "3)Update car details" << endl;

        while (1)
        {
            cin >> incar;
            if (incar < 1 || incar > 3)
            {
                cout << endl
                     << "Invalid input, please try again" << endl;
                continue;
            }
            break;
        }
        switch (incar)
        {
        case 1:
            cout << "Enter car model" << endl;
            cin.ignore();
            getline(cin, car.model);
            cout << "Enter car condition";
            cin >> car.condition;
            cout << "Enter car price:" << endl;
            cin >> car.price;
            // cout<<"Enter due date(format DD/MM/YYYY)";
            // cin>>car.dueDate;
            car.dueDate = "DD/MM/YYYY";
            car.other_details = "Empty";
            car.status = "unrented";
            tempInt = stoi(getFirstElementOfLastRowInCSV("car.csv"));
            tempInt++;
            cout << tempInt;
            temp = to_string(tempInt);

            if (tempInt < 10)
            {
                car.ID = "000" + temp;
            }
            else if (tempInt < 100)
            {
                car.ID = "00" + temp;
            }
            else if (tempInt < 1000)
            {
                car.ID = "0" + temp;
            }
            else
            {
                car.ID = temp;
            }

            temp = car.ID + "," + car.model + "," + car.condition + "," + car.other_details + "," + car.status + "," + car.price + "," + car.dueDate;
            cout << temp;
            car.addcar(temp);
            break;
        case 2:
            cout << "Enter ID number of the car you want to remove:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid4DigitCarID(temp))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            car.deleteCar(temp);
            break;
        case 3:
            cout << "Enter ID number of the car you want to update:" << endl;
            while (1)
            {
                cin >> temp;
                if (!valid4DigitCarID(temp))
                {
                    cout << endl
                         << "Invalid input, please try again" << endl;
                    continue;
                }
                break;
            }
            car.updateCarDetails(temp);
            break;
        default:
            break;
        }
        break;
        ////
    case 4:
        cout << endl
             << "Enter ID of car returned by customer" << endl;
        while (1)
        {
            cin >> temp;
            if (!valid4DigitCarID(temp))
            {
                cout << endl
                     << "Invalid input, please try again" << endl;
                continue;
            }
            break;
        }
        car.returnCarC(carID);
        break;
    case 5:
        flag == 1;
    default:
        break;
    }
}

void iAmCustomer()
{
    customer c;
    car cr;
    string carToRent;
    int inc, rent, temp;
    cout << "Please enter your customerID" << endl;
    	while (1)
         {
            cin >> c.ID;
            if (!valid4DigitID2(c.ID))
		 {
			cout<<endl<<"Invalid input, please try again"<<endl;
			continue;
		 }
         break;
         }

    cout << c.viewCustomerDetails(c.ID);
    cout << endl
         << "What would you like to do:" << endl
         << "1)See cars you have rented:" << endl
         << "2)See available cars:" << endl;
    cin >> inc;
    switch (inc)
    {
    case 1:
        cout << "Here are your cars:" << endl;
        cr.showCarsByOwners(c.ID);
        cout << endl
             << "Would you like to see due dates:" << endl
             << "1)Yes" << endl
             << "2)No";
        cin >> temp;
        if (temp == 1)
        {
            cr.showDueDate(c.ID);
        }

        break;
    case 2:
        cout << "Here are all available cars:" << endl;
        cr.showCarsByOwners("unrented");
        cout << endl
             << "would you like to rent a car:" << endl
             << "1)Yes" << endl
             << "2)No" << endl;
        cin >> rent;
        if (rent == 1)
        {
            cout << "Enter ID of car you would like to rent";
            cr.showAvaCarDetalils(0);
        }
        cin >> carToRent;
        cr.sentRequestC(c.ID, carToRent);
    default:
        break;
    }
}

void iAmEmployee()
{
    employee e;
    car cr;
    string carToRent;
    int inc, rent, temp;
    cout << "Please enter your employeeID" << endl;
      while (1)
         {
            cin >> e.ID;
            if (!valid3DigitID2(e.ID))
		 {
			cout<<endl<<"Invalid input, please try again"<<endl;
			continue;
		 }
         break;
         }
    cout << e.viewEmployeeDetails(e.ID);
    cout << endl
         << "What would you like to do:" << endl
         << "1)See cars you have rented:" << endl
         << "2)See available cars:" << endl;
    switch (inc)
    {
    case 1:
        cout << "Here are your cars:" << endl;
        cr.showCarsByOwners(e.ID);
        cout << endl
             << "Would you like to see due dates:" << endl
             << "1)Yes" << endl
             << "2)No";
        cin >> temp;
        if (temp == 1)
        {
            cr.showDueDate(e.ID);
        }
        break;
    case 2:
        cout << "Here are all available cars:" << endl;
        cr.showCarsByOwners("unrented");
        cout << "would you like to rent a car:"
             << "1)Yes" << endl
             << "2)No" << endl;
        cin >> rent;
        if (rent == 1)
        {
            cout << "Enter ID of car you would like to rent";
            cr.showAvaCarDetalils(1);
        }
        cin >> carToRent;
        cr.sentRequestE(e.ID, carToRent);
    default:
        break;
    }
}

int main()
{
    int designation;
    cout << "Chose your designation(enter respective number):" << endl
         << "1)Manager" << endl
         << "2)Customer" << endl
         << "3)Employee" << endl;
    cin >> designation;
    switch (designation)
    {
    case 1:
        iAmManager();
        break;
    case 2:
        iAmCustomer();
        break;
    case 3:
        iAmEmployee();

    default:
        break;
    }
}
