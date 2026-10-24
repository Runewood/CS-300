
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <cctype>

using namespace std;

// Course object
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Hash table using separate chaining
const int tableSize = 101;
vector<list<Course>> courses(tableSize);

// Here we make a new function to handle any unexpected characters in the CSV and trim it
string trim(string text) {
    // Left trim 
    text.erase(0, text.find_first_not_of(" \n\r\t"));

    // Right trim 
    text.erase(text.find_last_not_of(" \n\r\t") + 1);
    
    // If the string is empty after trimming return an empty string
    if (text.empty()) {
        return "";
    }
    return text;
}

// Convert course number into bucket index
int hashCourseNum(string courseNumber) {
    return std::hash<string>{}(courseNumber) % tableSize;
}

// Insert course into hash table
void insertCourse(Course course) {
    int bucket = hashCourseNum(course.courseNumber);

    // The list automatically handles collisions
    courses[bucket].push_back(course);
}

// Find course in hash table
Course* findCourse(string courseNumber) {
    // Set the bucket variable to course number hash value
    int bucket = hashCourseNum(courseNumber);

    // Iterate through to find the course in the bucket
    for (Course& course : courses[bucket]) {
        if (course.courseNumber == courseNumber) {
            return &course;
        }
    }

    return nullptr;
}

// Load the CSV into hash table 
void loadCourse(string filename) {

    // First we clear the any previously loaded courses
    courses = vector<list<Course>>(tableSize);

    // Open the file
    ifstream file(filename);

    // If file is not open
    if (!file.is_open()) {
        // Return error
        cout << "Error opening file!" << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        Course course;
        string prerequisite;

        // Parse required fields 
        getline(ss, course.courseNumber, ',');
        getline(ss, course.courseTitle, ',');

        // Trim any leading or trailing whitespace for edge cases
        course.courseNumber = trim(course.courseNumber);
        course.courseTitle = trim(course.courseTitle);

        if (course.courseNumber.empty() ||
            course.courseTitle.empty()) {
            cout << "Error: Invalid course data!" << endl;
            courses = vector<list<Course>>(tableSize);
            return;
        }

        // Parse the remaining prerequisites
        while (getline(ss, prerequisite, ',')) {
            // Here we call trim again
            prerequisite = trim(prerequisite);

            // This handles empty/no prerequisites 
            if (!prerequisite.empty()) {
                course.prerequisites.push_back(prerequisite);
            }
        }

        // Insert directly into hash table
        insertCourse(course);
    }

    // Validate prerequisites
    for (const auto& bucket : courses) {
        for (const Course& course : bucket) {
            for (const string& prereq : course.prerequisites) {

                // Checking for missing prerequisites and printing errors
                if (findCourse(prereq) == nullptr) {
                    cout << "Invalid prerequisite: [" << prereq << "]" << endl;
                    cout << "Referenced by: " << course.courseNumber << endl;
                    courses = vector<list<Course>>(tableSize);
                    return;
                }
            }
        }
    }

    file.close();
    cout << "Courses loaded successfully!" << endl;
}

// Search course and display its information
void searchCourse(string courseNumber) {
    Course* course = findCourse(courseNumber);

    // If course not found print out error
    if (course == nullptr) {
        cout << "Course not found!" << endl;
        return;
    }

    // Print out course information
    cout << course->courseNumber << ", "
         << course->courseTitle << endl;

    // Print out prerequisites
    cout << "Prerequisites:" << endl;

    // If no prerequisites print out none
    if (course->prerequisites.empty()) {
        cout << "None" << endl;
    }

    // Iterate through prerequisites and print them out
    for (const string& prereq : course->prerequisites) {
        Course* prerequisiteCourse = findCourse(prereq);

        // If the prerequisite course is found, print its information
        if (prerequisiteCourse != nullptr) {
            cout << prerequisiteCourse->courseNumber
                 << ", " << prerequisiteCourse->courseTitle
                 << endl;
        }
    }
}

// Print courses in alphanumeric order
void printCourseList() {
    vector<Course> sortedCourses;

    // Copy courses from each hash table bucket
    for (const auto& bucket : courses) {
        for (const Course& course : bucket) {
            sortedCourses.push_back(course);
        }
    }

    // Sort by course number
    sort(sortedCourses.begin(), sortedCourses.end(),
        [](const Course& a, const Course& b) {
            return a.courseNumber < b.courseNumber;
        });

    // Print sorted courses
    for (const Course& course : sortedCourses) {
        cout << course.courseNumber << ", "
             << course.courseTitle << endl;
    }
}

// Main program menu
int main() {
    int choice = 0;
    string filename;
    string courseNumber;
    bool loaded = false;

    cout << "-------------------------" << endl;
    cout << "ABCU course planner" << endl;
    cout << "-------------------------" << endl;
    cout <<" Please select a menu option:" << endl;

    while (choice != 9) {
        cout << "\n1. Load Course Data from CSV file" << endl;
        cout << "2. Print Course List in alphanumeric order" << endl;
        cout << "3. Print Course Information" << endl;
        cout << "9. Exit" << endl;
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input!" << endl;
            continue;
        }

        switch (choice) {
        case 1:
            cout << "Enter exact filename (CSV only): ";
            getline(cin >> ws, filename);

            // Here we will add .csv if user does not provide it
            if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".csv") {
                filename += ".csv";
            }
            
            loaded = false;
            loadCourse(filename);
            
            // Here we check if the courses actually loaded into the hash table. If not, we set loaded to false.
            for (const auto& bucket : courses) {
                if (!bucket.empty()) {
                    loaded = true;
                    break;
                }
            }

            break;

        case 2:
            if (loaded) {
                printCourseList();
            }
            else {
                cout << "Please load the file first!" << endl;
            }
            break;

        case 3:
            if (loaded) {
                cout << "Enter course number: ";
                cin >> courseNumber;

                // Case normalization for ease of use for user
                for (char& letter : courseNumber) {
                    letter = toupper(letter);
                }

                searchCourse(courseNumber);
            }
            else {
                cout << "Please load the file first!" << endl;
            }
            break;

        case 9:
            cout << "Goodbye!"
                 << endl;
            break;

        default:
            cout << "Invalid menu option!" << endl;
        }
    }

    return 0;
}