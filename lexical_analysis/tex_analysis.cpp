#include<iostream>
#include<string>
#include<vector>
using namespace std;



bool flag=true;
int cur_line=0;//正在解析的行数
struct  word_encode_table{
    int id;
    string str;
};


class analysis_tex
{
    public:
    FILE *file;//源代码文件
    FILE *file_err;//保存错误信息
    FILE *file_dyd;//保存二元式
    char cha;//记录当前字符
    analysis_tex(const char *filename,const char *err_filename,const char *dyd_filename){
        cout<<name<<endl;
        int err;
        int err1;
        int dyd;
        //创建二元式文件和错误信息文件
        err1 = fopen_s(&file_err, err_filename, "w");
        dyd = fopen_s(&file_dyd,dyd_filename,"w");
        // 打开文件
        err = fopen_s(&file, filename, "r");
        if (err != 0||err1!=0||dyd!=0) {
            printf("can't open the file\n");
            exit(1);
        }
    }

    ~analysis_tex(){
        fclose(file);
        fclose(file_dyd);
        fclose(file_err);
    }
    void solution()
    {
        while(flag)
        {
            string result=word();
            if(result!="")
            cout<<result<<endl;
        }
    }

    private:

    const string name= "zlj's lexical analysis class";
    word_encode_table key_table[9]={ {1,"begin"}, {4,"if"},  {2,"end"}, {7,"function"}, {5,"then"},  {8,"read"},  {3,"integer"},  {6,"else"},  {9,"write"} };
    vector<word_encode_table> symbol_table;
    vector <int> constant_table;
    string token="";

    bool letter(){
        if((cha>='A' && cha<='Z')||(cha>='a' && cha<='z'))
        return true;
        return false;
    }

    bool digit(){
        if((cha>='0' && cha<='9'))
        return true;
        return false;
    }

    void getnbc(){
        cha=fgetc(file);//getchar();
        while(cha==' '|| cha=='\n')
        {
            if(cha=='\n')
            {
            fprintf(file_dyd,"%16s %d\n","EOLN",24);
            cur_line=cur_line+1;
            }
            cha=fgetc(file);
        }
    }

    void retract()//字符回退
    {
        fseek(file, -1, SEEK_CUR);
        cha='\0';
    }

    void concat()
    {
        token.push_back(cha);
    }

    int constant()
     {
        int size = constant_table.size();
        int tmp = stoi(token);//字符串转数值型
        for (int i = 0; i < size;i++) {
            if (tmp == constant_table[i]) {
                return i;
            }
        }
        constant_table.push_back(tmp);
        return size;
    }

    int reserve()
    {
        for(int i=0;i<9;i++)
        {
            if(token==key_table[i].str)
            {
                return key_table[i].id;
            }
        }
        return -1;
    }
    
    int buildlist(string token)
    {
        int size=symbol_table.size();
        for(int i=0;i<size;i++)
        {
            if(token==symbol_table[i].str)
            {
                return symbol_table[i].id;
            }
        }

        word_encode_table temp={size,token};
        symbol_table.push_back(temp);
        return size;
    }

    void err_printf(string str)
    {
        fprintf(file_err,"LINE:%d  %s\n",cur_line,str.c_str());
        cout <<"LINE:" << cur_line << "  " << str << endl;//暂时输出错误消息
    }
   
   void print(string str,int id)
   {
    fprintf(file_dyd,"%16s %2d\n",str.c_str(),id);
   }

   string word()
   {
    token="";
    getnbc();
    if (cha == EOF) 
    {
        print("EOF",25);
        flag = false;
        return "";
    }
    concat();
    if(letter())
    {
        cha=fgetc(file);
        while(letter()||digit())
        {
            concat();
            cha=fgetc(file);
        };
        retract();
        int i=reserve();
        //标识符
        if (token.size() > 16) {
            err_printf("标识符长度溢出");
        }
        //写二元式到文件
        string substring = token.substr(0, 16);
        if(i==-1)
        {
            print(substring,10);
            int symbol_id=buildlist(substring);
            return string(16-substring.size(),' ')+substring+' '+to_string(10)+' '+to_string(symbol_id);
        }
        else
        {
            print(token,i);
            return string(16-token.size(),' ')+token+' '+to_string(i);
        }
    }
    else if(digit())
    {
        bool letter_flag=0;
        cha=fgetc(file);
        while(digit()||letter())
        {
            concat();
            cha=fgetc(file);
            if(letter())
            letter_flag=1;
        }
        if(!letter_flag)
        {
            int constant_id=constant();
            retract();
            print(token,11);
            return string(16-token.size(),' ')+token+' '+to_string(11)+' '+to_string(constant_id);
        }
        else
        {
            err_printf("error constant number");
            retract();
            return "";
        }
    }
    else if(cha=='=')
    {
        print(token,12);
        return string(16-token.size(),' ')+token+' '+to_string(12);
    }
    else if(cha=='-')
    {
        print(token,18);
        return string(16-token.size(),' ')+token+' '+to_string(18);
    }
    else if(cha=='*')
    {
        print(token,19);
        return string(16-token.size(),' ')+token+' '+to_string(19);
    }
    else if(cha=='(')
    {
        print(token,21);
        return string(16-token.size(),' ')+token+' '+to_string(21);
    }
    else if(cha==')')
    {
        print(token,22);
        return string(16-token.size(),' ')+token+' '+to_string(22);
    }
    else if(cha=='<')
    {
        cha=fgetc(file);
        if(cha=='=')
        {
            concat();
            print(token,14);
            return  string(16-token.size(),' ')+token+' '+to_string(14);
        }
        else if(cha=='>')
        {
            concat();
            print(token,13);
            return  string(16-token.size(),' ')+token+' '+to_string(13);
        }
        else
        {
            print(token,15);
            retract();
            return  string(16-token.size(),' ')+token+' '+to_string(15);
        }
    }
    else if(cha=='>')
    {
        cha=fgetc(file);
        if(cha=='=')
        {
            concat();
            print(token,16);
            return  string(16-token.size(),' ')+token+' '+to_string(16);
        }
        else
        {
            print(token,17);
            retract();
            return  string(16-token.size(),' ')+token+' '+to_string(17);
        }
    }
    else if(cha==':')
    {
        cha=fgetc(file);
        concat();
        if(cha=='=')
        {
            print(token,20);
            return  string(16-token.size(),' ')+token+' '+to_string(20);
        }
        else
        {
            err_printf("冒号不匹配");
            retract();
            return "";
        }
    }
    else if(cha==';')
    {
        print(token,23);
        return string(16-token.size(),' ')+token+' '+to_string(23);
    }
    else
    err_printf(string("error token ")+cha);
    return "";
   }

};

int main(){
    const char* src_filename = "C:/Users/18397/Desktop/bianyi/src.pas";
    const char* err_filename = "C:/Users/18397/Desktop/bianyi/src.err";
    const char* dyd_filename = "C:/Users/18397/Desktop/bianyi/src.dyd";
    analysis_tex tex(src_filename,err_filename,dyd_filename);

    tex.solution();
    return 0;
}