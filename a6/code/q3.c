struct Student {
      int sid;
      int grade[4];
      int average;      // this is computed by your program
    };

    int n;              // number of students
    int m;              // you store the median student's id here
    struct Student* s;  // a dynamic array of n students


void question3() {
    // compute and store averages for each student
    for (int i = 0; i < n; i++)
        s[0].average = (s[0].grade[0] + s[0].grade[1] + s[0].grade[2] + s[0].grade[3]) / 4;

    // sort students by ascending average
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (s[j].average < s[i].average) {
                int t = s[j];
                s[i] = s[j];
                s[j] = t;
            }
}