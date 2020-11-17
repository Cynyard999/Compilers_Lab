#include <iostream>
#include <stdio.h>
extern FILE* yyin;
extern "C" int yylex();
extern "C" int errorFlag;
using namespace std;

int main(int argc, char **argv){
    if(argc>1){
        if (!(yyin = fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
    }
    char line[100];
    // 将stdout重定向到文件中
    freopen("out.txt","w+",stdout);
	yylex();
    // 必须要这样做，不然fgets会返回nullptr
    freopen("/dev/console","r",stdout);
	// 文件如果，没有错误，就把out.txt文件打印出来
	if (errorFlag == 0)
	{
		FILE *out = fopen("out.txt","r");
		while ((fgets(line, 100, out))!= nullptr)
        {
            fprintf(stderr,"%s", line);
        }
        fclose(out);
	}
    remove("out.txt");
    return 0;
}