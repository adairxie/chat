/* permuatation the string */
/* author xiehui */
/* runhxie@gmail.com */
/* 2016-09-01 */
/*#include <iostream>
#include <string>
#include <vector>
*/

void permutationHelp(char *pStr, char *pBegin, std::vector<std::string>& result)
{
	if (*pBegin == '\0') {
		//printf("%s\n", pStr);
		result.push_back(std::string(pStr));
		return;
	}
    else {			
		for (char *p = pBegin; *p != '\0'; p++) {
			char tmp = *p;
			*p = *pBegin;
			*pBegin = tmp;
			permutationHelp(pStr, pBegin+1, result);
	
			tmp = *p;
			*p = *pBegin;
			*pBegin = tmp;
		}
	}
}


std::vector<std::string> permutation(char *pStr)
{
	if (pStr == NULL)
		return std::vector<std::string>();
	std::vector<std::string> result;
	permutationHelp(pStr, pStr, result);
	return result;
}

/*int main()
{
	char str[] = "abcdefgh";
	std::vector<std::string> ret;
	ret = permutation(str);
	if (!ret.empty())
		std::cout << "size: " << ret.size() << std::endl;
	return 0;
}*/
