#ifndef HANDLESTRING_H
#define HALDLESTRING_H


/*
* Find Case Insesitive Sub String in a given substring
*/
size_t ifind(std::string data, std::string toSearch, size_t pos=0);

bool icompare_pred(unsigned char a, unsigned char b);

/*
* Compare Case Insesitive Sub String in a given substring
*/

bool icompare(std::string const &a, std::string const &b);

std::string trim(const std::string& str, const std::string& whitespace = "\t");
std::string reduce(const std::string& str, const std::string& fill = " ", \
const std::string& whitespace = "\t");
bool stob(std::string data);
char* upperCase(char *str, int len);

#endif    // HANDLESTRING_H

