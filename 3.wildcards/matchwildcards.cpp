#include <iostream>
#include <string>
#include <stdio.h>
#include <stdbool.h>

bool match(const char *first, const char * second)
{
    // If we reach at the end of both strings, we are done
    if (*first == '\0' && *second == '\0')
        return true;

    // Make sure that the characters after '*' are present
    // in second string. This function assumes that the first
    // string will not contain two consecutive '*'
    if (*first == '*' && *(first+1) != '\0' && *second == '\0')
        return false;

    // If the first string contains '?', or current characters
    // of both strings match
    if (*first == '?' || *first == *second)
        return match(first+1, second+1);

    // If there is *, then there are two possibilities
    // a) We consider current character of second string
    // b) We ignore current character of second string.
    if (*first == '*')
        return match(first+1, second) || match(first, second+1);

    return false;
}

bool matchwildcards(std::string inputstring, std::string searchpattern)
{
    return match(searchpattern.c_str(), inputstring.c_str());
}


// A function to run test cases
void test(std::string first, std::string second)
{
    matchwildcards(first, second) ? std::cout << "Yes" << std::endl : std::cout << "No" << std::endl;
}

int main()
{
    std::cout << "test(\"tests\", \"t*ts\") - ";
    test("tests", "t*ts" ); // Yes
    std::cout << "test(\"wildcardstest\", \"wi?dc*\") - ";
    test("wildcardstest", "wi?dc*"); // Yes
    std::cout << "test(\"gee\", \"g*m\") - ";
    test("gee", "g*m");  // No because 'k' is not in second
    std::cout << "test(\"sqrt\", \"*qrt\") - ";
    test("sqrt", "*qrt"); // No because 't' is not in first
    std::cout << "test(\"abcdhghgmnh\", \"abc*mnh\") - ";
    test("abcdhghgmnh", "abc*mnh"); // Yes
    std::cout << "test(\"abcd\", \"abc*c?d\") - ";
    test("abcd", "abc*c?d"); // No because second must have 2 instances of 'c'
    std::cout << "test(\"abcd\", \"*c*d\") - ";
    test("abcd", "*c*d"); // Yes
    std::cout << "test(\"abcd\", \"*?c*d\") - ";
    test("abcd", "*?c*d"); // Yes
    
    return 0;
}
