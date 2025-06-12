#include <iostream>
#include <vector>

using namespace std;
// 注释中e代表空串
class syntax_analysis
{
public:
	struct WORD
	{ // 单词二元式
		string name;
		int flag;
	};

	struct Par_Table
	{ // 变量表
		string vname;
		string vproc;
		int vkind; // 0,1
		string vtype;
		int vlev;
		int vadr;
		bool defined; //	false表示未定义，true表示已定义
	};

	struct Pro_Table
	{ // 过程分析表
		// 过程名pname: char(16),用string代替
		// 过程类型ptype: types
		// 过程层次plev: int
		// 第一个变量在变量表中的位置fadr: int
		// 最后一个变量在变量表中的位置ladr: int
		string name;
		string ptype;
		int plev;
		int fadr;
		int ladr;
	};

	syntax_analysis(const char *tex_err_filename, const char *dyd_filename, const char *dys_filename, const char *var_filename, const char *pro_filename, const char *syntax_err_filename)
	{
		cout << "zlj's syntax_analysis has been launched" << endl;
		int err_tex = fopen_s(&file_tex_err, tex_err_filename, "r");
		int err_dyd = fopen_s(&file_dyd, dyd_filename, "r");
		int err_dys = fopen_s(&file_dys, dys_filename, "w");
		int err_var = fopen_s(&file_var, var_filename, "w");
		int err_pro = fopen_s(&file_pro, pro_filename, "w");
		int err_syntax = fopen_s(&file_syntax_err, syntax_err_filename, "w");
		if (err_dyd != 0 || err_tex != 0 || err_dys != 0 || err_var != 0 || err_pro != 0 || err_syntax != 0)
		{
			printf("打开文件失败\n");
			exit(1);
		}
		char tmp = fgetc(file_tex_err);
		if (tmp != EOF)
		{
			printf("词法分析不正确\n");
			exit(-1);
		}
	}
	~syntax_analysis()
	{
		fclose(file_tex_err);
		fclose(file_dyd);
		fclose(file_dys);
		fclose(file_var);
		fclose(file_pro);
		fclose(file_syntax_err);
	}

	void solution()
	{
		syntax_analize();

		write_var_pro();
	}

private:
	vector<string> cur_pro;		 // 存储当前过程名
	vector<Par_Table> par_table; // 变量名表
	vector<Pro_Table> pro_table; // 过程表
	FILE *file_dyd;
	FILE *file_tex_err;
	FILE *file_dys;
	FILE *file_var;
	FILE *file_pro;
	FILE *file_syntax_err;
	int cur_line = 0;		 // 表示当前源码中的行数
	int cur_line_in_dyd = 0; // 表示当前dyd中的行数
	int cur_line_in_dys = 0; // 表示当前dys打印到第几行
	WORD cur_word;
	size_t current_POS = 0;	 // 当前位置
	size_t previous_POS = 0; // 上一个位置
	bool error = 0;
	bool is_subpro = 0; // 标记是不是子过程

	//过程表中最后一个变量在变量表中的位置处理
	void compute_laddr(int vlev,string vproc)
	{
		int size=pro_table.size();
		for(int i=0;i<size;i++)
		{
			if(pro_table[i].plev==vlev && pro_table[i].name==vproc)
			pro_table[i].ladr++;
		}
	}
	// 回退一行
	void back_mid()
	{
		fseek(file_dyd, -20, SEEK_CUR);

		if (cur_line_in_dyd != 0)
		{
			cur_line_in_dyd--;
		}
		cur_word.name="";
		cur_word.flag=-1;
	}
	// 回退到非换行符的上一行
	void back()
	{
		back_mid();
		WORD tmp;
		char str[20] = "";

		char buf[128];
		if (fgets(buf, sizeof(buf), file_dyd))
		{
			if (sscanf(buf, "%16s %d", str, &tmp.flag) == 2) {
				cur_line_in_dyd++;
				tmp.name = str;
			}
		}
		back_mid();
		while (tmp.flag == 24)
		{
			back_mid();
			cur_line--;
		char buf[128];
		if (fgets(buf, sizeof(buf), file_dyd))
		{
			if (sscanf(buf, "%16s %d", str, &tmp.flag) == 2) {
				cur_line_in_dyd++;
				tmp.name = str;
			}
		}
			back_mid();
		}
	}
	void get_word()
	{
		char buf[128];
		char str[20] = "";
		int flag = -1;

		if (fgets(buf, sizeof(buf), file_dyd)) {
			// 用sscanf从这一行解析
			if (sscanf(buf, "%16s %d", str, &flag) == 2) {
				cur_word.name = str;
				cur_word.flag = flag;

				if (cur_line_in_dyd == cur_line_in_dys) {
					fprintf(file_dys, "%16s %2d\n", cur_word.name.c_str(), cur_word.flag);
					cur_line_in_dys++;
				}
				cur_line_in_dyd++;
			} else {
				cur_word.name = "";
				cur_word.flag = -1;
			}
		} else {
			cur_word.name = "";
			cur_word.flag = -1;
		}
	}

	void get_nword()
	{ // 读取一个非换行符的二元式
		get_word();
		if (cur_word.flag == 24)
		{ // 换行符
			cur_line++;
			get_word();
		}
	}

	void advance()
	{
		get_nword();
	}

	int find_pro_adr(Pro_Table p)
	{
		int size = pro_table.size();
		for (int i = 0; i < size; i++)
		{
			if (p.name == pro_table[i].name)
			{
				return i;
			}
		}
		return -1;
	}

	int find_par_adr(Par_Table p)
	{
		int size = par_table.size();
		for (int i = 0; i < size; i++)
		{
			if (p.vname == par_table[i].vname && p.vproc == par_table[i].vproc && p.vlev == par_table[i].vlev)
			{
				return i;
			}
		}
		return -1;
	}

	int param(string fun_name)
	{
		advance(); // 处理函数参数
		Par_Table tmp;

		// 将函数参数放入变量表

		tmp.vname = cur_word.name;
		tmp.vproc = fun_name;
		tmp.vkind = 1;
		tmp.vtype = "integer";
		tmp.vlev = cur_pro.size();
		tmp.defined = false;
		tmp.vadr = find_par_adr(tmp);
		if (tmp.vadr == -1)
		{
			tmp.vadr = par_table.size();
			par_table.push_back(tmp);
			compute_laddr(tmp.vlev,tmp.vproc);
		}

		return tmp.vadr;
	}

	void add_fun_name(string fun_name, int index) // 将定义的函数名加入过程名表
	{
		int size = pro_table.size();
		for (int i = 0; i < size; i++)
		{
			if (pro_table[i].name == fun_name && pro_table[i].plev == cur_pro.size())
			{
				// 函数重定义(在同一个层次重定义)
				printf("LINE:%d 函数 %s 重定义\n", cur_line + 1, fun_name.c_str());
				fprintf(file_syntax_err, "LINE:%d 函数 %s 重定义\n", cur_line + 1, fun_name.c_str());
				error = 1;
				return;
			}
		}
		Pro_Table tmp;
		tmp.name = fun_name;
		tmp.ptype = "integer";
		tmp.plev = cur_pro.size();
		tmp.fadr = index;
		tmp.ladr = index;
		pro_table.push_back(tmp);
	}

	void F_C(string name)
	{
		// F_C->e|(算术表达式)
		//  因为函数调用和变量都是标识符，提取公共左因子
		string fun_name = cur_word.name;

		advance();
		if (cur_word.name == "(")
		{

			Pro_Table tmp;
			tmp.name = fun_name;

			int i = find_pro_adr(tmp);
			if (i == -1)
			{
				printf("LINE:%d 函数 %s 未定义\n", cur_line + 1, fun_name.c_str());
				fprintf(file_syntax_err, "LINE:%d 函数 %s 未定义\n", cur_line + 1, fun_name.c_str());
				error = 1;
			}

			arith_exp();
			advance();
			if (cur_word.name != ")")
			{
				printf("LINE:%d 括号不匹配\n", cur_line + 1);
				fprintf(file_syntax_err, "LINE:%d 括号不匹配\n", cur_line + 1);
				error = 1;
				back();
			}
		}
		else
		{
			Par_Table tmp;
			tmp.vname = name;
			tmp.vlev = cur_pro.size();
			tmp.vproc = cur_pro.back();
			int i = find_par_adr(tmp);
			if (i == -1 || !par_table[i].defined)
			{
				printf("LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
				fprintf(file_syntax_err, "LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
				error = 1;
			}
			back();
		}
	}
	void yinzi()
	{ //<因子>→<变量>│<常数>│<函数调用>
		// 提取公共左因子
		// 因子->常数|标识符F_C
		//  因为函数调用和变量都是标识符，提取公共左因子
		// F_C->e|(算术表达式)
		advance();
		if (cur_word.flag == 10)
		{
			// 当前为标识符号
			// 需要进一步判断是否为函数名字,进一步判断是否为函数调用
			string name = cur_word.name;

			F_C(name); // function call
		}
		else if (cur_word.flag == 11)
		{
			// 当前为常数,暂时打印出来
			// printf("%s\n",cur_word.name.c_str());
		}
	}
	void X()//X表示项'
	{
		advance();
		if (cur_word.name == "*")
		{
			yinzi();
			X();
		}
		else
		{
			back();
		}
		// e
	}

	void xiang()
	{
		// 消除左递归后
		// 项->因子 X
		// X->*因子 X|e
		yinzi(); // 判断是否为因子
		X();
	}

	void A() //减法运算处理，A算数表达式'
	{ // A-> -项A | e
		advance();
		if (cur_word.name == "-")
		{
			xiang();
			A();
		}
		else
		{
			back();
		}

		// e
	}

	void arith_exp()
	{ // 算数表达式->项A
		xiang();
		A();
	}

	void condi_exp()
	{ // 判断条件表达式cur_word后的
		arith_exp();
		advance();
		if (cur_word.name == "<" || cur_word.name == "<=" || cur_word.name == ">" || cur_word.name == ">=" || cur_word.name == "=" || cur_word.name == "<>")
		{
			// 表明为正确的关系运算符
		}
		arith_exp();
	}
	void exe_line()
	{
		advance();

		if (cur_word.name == "read")
		{
			advance();
			// 判断是否为变量
			if (cur_word.name == "(")
			{
				advance(); // 读取变量名
				// 检查变量
				Par_Table tmp;
				tmp.vname = cur_word.name;
				tmp.vlev = cur_pro.size();
				tmp.vproc = cur_pro.back();
				int i = find_par_adr(tmp);
				if (i == -1 || !par_table[i].defined)
				{
					printf("LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
					fprintf(file_syntax_err, "LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
					error = 1;
				}
				advance();
				if (cur_word.name == ")")
				{

					// read语句
				}
				else
				{
					printf("LINE:%d 括号不匹配\n", cur_line + 1);
					fprintf(file_syntax_err, "LINE:%d 括号不匹配\n", cur_line + 1);
					error = 1;
					back();
				}
			}
		}
		else if (cur_word.name == "write")
		{
			advance();
			// 判断是否为变量
			if (cur_word.name == "(")
			{
				advance();
				Par_Table tmp;
				tmp.vname = cur_word.name;
				tmp.vlev = cur_pro.size();
				tmp.vproc = cur_pro.back();
				int i = find_par_adr(tmp);
				if (i == -1 || !par_table[i].defined)
				{
					printf("LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
					fprintf(file_syntax_err, "LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
					error = 1;
				}
				advance();
				if (cur_word.name == ")")
				{
					// write语句
				}
				else
				{
					printf("LINE:%d 括号不匹配\n", cur_line + 1);
					fprintf(file_syntax_err, "LINE:%d 括号不匹配\n", cur_line + 1);
					error = 1;
					back();
				}
			}
		}
		else if (cur_word.name == "if")
		{				 // 条件语句
			condi_exp(); // 条件表达式
			advance();
			if (cur_word.name == "then")
			{
				// 执行语句
				exe_line();
				advance();
				if (cur_word.name == "else")
				{
					// 执行语句
					exe_line();
				}
			}
		}
		else if (cur_word.name == "integer")
		{
			printf("LINE:%d 说明语句位置错误\n", cur_line + 1); 
			fprintf(file_syntax_err, "LINE:%d 说明语句位置错误\n", cur_line + 1);
			error = 1;
			int line = cur_line;
			while (cur_word.name != ";" && line == cur_line)
			{ //||  到下一行检测
				advance();
			}
			back();
			return;
		}
		else if (cur_word.flag == 10)
		{   // 赋值语句
			// 此时cur_word应该是一个变量
			// 判断变量是否定义
			if (is_subpro == 0)
			{
				string fun_or_varity_name=cur_word.name;

				advance();
				if(cur_word.name=="(")
				{
					Pro_Table tmp;
					tmp.name=fun_or_varity_name;
					int i = find_pro_adr(tmp);
					if (i == -1)
					{
						printf("LINE:%d 函数 %s 未定义\n", cur_line + 1, fun_or_varity_name.c_str());
						fprintf(file_syntax_err, "LINE:%d 函数 %s 未定义\n", cur_line + 1, fun_or_varity_name.c_str());
						error = 1;
					}

					arith_exp();
					advance();
					if (cur_word.name != ")")
					{
						printf("LINE:%d 括号不匹配\n", cur_line + 1);
						fprintf(file_syntax_err, "LINE:%d 括号不匹配\n", cur_line + 1);
						error = 1;
						back();
					}
				}
				else 
				{
					Par_Table tmp;
					tmp.vname =fun_or_varity_name;
					tmp.vlev = cur_pro.size();
					tmp.vproc = cur_pro.back();
					int i = find_par_adr(tmp);
					if (i == -1 || !par_table[i].defined)
					{
						printf("LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
						fprintf(file_syntax_err, "LINE:%d 变量 %s 未定义\n", cur_line + 1, tmp.vname.c_str());
						error = 1;
					}
					if (cur_word.name == ":=")
						{
							arith_exp();
						}		
				}
				int line=cur_line;
				advance();
               if(cur_line!=line)
			     {
					back();
					back();
				advance();}
				while(cur_word.name!=";"&& line==cur_line)
				{
					advance();
				};
				if(cur_word.name==";")
				back();
				return;
			}
			else
			{
				Pro_Table tmp;
				tmp.name = cur_word.name;
				int i = find_pro_adr(tmp);
				if (i == -1)
				{
					printf("LINE:%d 函数 %s 未定义\n", cur_line + 1, tmp.name.c_str());
					fprintf(file_syntax_err, "LINE:%d 函数 %s 未定义\n", cur_line + 1, tmp.name.c_str());
					error = 1;
				}
			}
			advance();
			if (cur_word.name == ":=")
			{
				// 此时期待一个算数表达式
				arith_exp(); // 判断是否为算数表达式
			}
			else
			{

				back();
			}
		}
		else
		{
			printf("LINE:%d 错误的执行语句 \n", cur_line + 1);
			fprintf(file_syntax_err, "LINE:%d 错误的执行语句\n", cur_line + 1);
			error = 1;
			back();
		}
	}
	void E()
	{ // E->;执行语句E|e
		advance();
		if (cur_word.name == ";")
		{
			exe_line();
			E();
		}
		else
		{
			// 匹配e
			back();
		}
	}

	void exe_table()
	{				// 执行语句表->执行语句E  E->;执行语句E|e (E代表执行语句表')
		exe_line(); // 处理执行语句
		E();
	}
	void bold()
	{ // 函数体->分程序/begin 说明语句表;执行语句表 end
		sub_P();
	}

	void F()
	{ // F->变量|function 标识符 （参数）;
		advance();
		if (cur_word.name == "function")
		{ // 处理函数说明
			advance();
			string fun_name = cur_word.name;
			cur_pro.push_back(cur_word.name);
			advance();
			if (cur_word.name == "(")
			{
				int index = param(fun_name); // 处理函数参数
				// index表示参数在变量名表中的位置
				advance();
				if (cur_word.name == ")")
				{
				}
				else
				{
					printf("LINE:%d 括号不匹配\n", cur_line + 1);
					fprintf(file_syntax_err, "LINE:%d 括号不匹配\n", cur_line + 1);
					error = 1;
					back();
				}
				// 将函数名加入过程名表
				add_fun_name(fun_name, index);//处理函数重定义
				advance();
				if (cur_word.name == ";")
				{
					is_subpro = 1;
					bold(); // 处理函数体
					is_subpro = 0;
					// 处理完毕后将函数名出栈
					// 函数定义结束
					cur_pro.pop_back();
				}
				else
				{
					printf("LINE:%d 函数体之前缺少\" ; \"\n", cur_line + 1);
					fprintf(file_syntax_err, "LINE:%d 函数体之前缺少\" ; \"\n", cur_line + 1);
					error = 1;
					back();
					is_subpro = 1;
					bold();
					is_subpro = 0;
					cur_pro.pop_back();
				}
			}
		}
		else if (cur_word.flag == 10)
		{ // cur_word为标识符,处理变量说明
			// var();//处理变量;//变量说明
			Par_Table tmp;
			tmp.vname = cur_word.name;
			tmp.vproc = cur_pro.back();
			tmp.vtype = "integer";
			tmp.vkind = 0;
			tmp.vlev = cur_pro.size();
			tmp.defined = true;
			int index = find_par_adr(tmp);
			if (index == -1)
			{
				tmp.vadr = par_table.size();
				par_table.push_back(tmp);
				compute_laddr(tmp.vlev,tmp.vproc);
			}
			else
			{
				if (!par_table[index].defined)
				{
					par_table[index].defined = true;
				}
				else
				// 变量重定义
				{
					printf("LINE:%d 变量 %s 重定义 \n", cur_line + 1, tmp.vname.c_str());
					fprintf(file_syntax_err, "LINE:%d 变量 %s 重定义 \n", cur_line + 1, tmp.vname.c_str());
					error = 1;
				}
			}
		}
		else if (cur_word.flag == 23)
		{
			printf("LINE:%d 说明语句未说明任何东西\n", cur_line + 1);
			fprintf(file_syntax_err, "LINE:%d 说明语句未说明任何东西\n", cur_line + 1);
			back();
		}
	}
	void T()//T表示说明语句表'
	{ // T->说明语句; T|e    说明语句->integer 说明语句'
		advance();

		if (cur_word.name == "integer")
		{
			F(); // 处理函数说明和变量说明
			advance();
			if (cur_word.name == ";")
			{
				// T();
			}
			else
			{
				printf("LINE:%d 说明语句后缺少\"; \"\n", cur_line + 1); // 缺少说明语句，word语句的意思
				fprintf(file_syntax_err, "LINE:%d 说明语句后缺少\"; \"\n", cur_line + 1);
				error = 1;
				back();
			}
			T();
		}
		else
		{
			back();
		}
	}
	void declar()
	{
		// B->说明语句;
		advance();

		if (cur_word.name != "integer")
		{
			printf("LINE:%d 缺少说明语句\n", cur_line);
			fprintf(file_syntax_err, "LINE:%d 缺少说明语句\n", cur_line);
			error = 1;
			back();
		}
		else
		{
			F();
			advance();
			if (cur_word.name != ";")
			{
				printf("LINE:%d 说明语句后缺少\"; \"\n", cur_line);
				fprintf(file_syntax_err, "LINE:%d 说明语句后缺少\"; \"\n", cur_line);
				error = 1;
				back();
			}
		}
	}
	void declar_table()
	{ // 说明语句表->BT   B->说明语句;   T->说明语句; T|e
		declar();
		T();
	}

	void sub_P()
	{ // 分程序->begin 说明语句表 执行语句表end;

		advance();
		int tmp = cur_line; // 记录当前行号
		int flag = 0;
		if (cur_word.name != "begin")
		{
			printf("LINE:%d 缺少 begin\n", cur_line + 1);
			fprintf(file_syntax_err, "LINE:%d 缺少 begin\n", cur_line + 1);
			error = 1;
			flag = 1;
			back();
		}
		declar_table();
		exe_table();
		advance();
		if (cur_word.name != "end")
		{
			if (flag == 1)
			{
				printf("LINE %d-LINE %d: 缺少begin和end\n", tmp, cur_line + 1);
				fprintf(file_syntax_err, "LINE %d-LINE %d: 缺少begin和end\n", tmp, cur_line + 1);
				error = 1;
			}
			else
			{
				printf("LINE:%d 缺少end与 %d 行的 begin 匹配\n", cur_line + 1, tmp);
				fprintf(file_syntax_err, "LINE:%d 缺少end与 %d 行的 begin 匹配\n", cur_line + 1, tmp);
				error = 1;
			}
			// 假设没有end匹配，默认有end,继续进行语法分析
			back();
		}
	}

	// 将变量表写回变量文件
	void write_var_pro()
	{
		fprintf(file_var,"%17s\t%5s\t%5s\t%10s\t%5s\t%5s\n", "vname", "vproc", "vkind", "vtype", "vlev", "vadr");
		fprintf(file_pro,"%17s\t%10s\t%5s\t%5s\t%5s\n", "pname", "ptype", "plev", "fadr", "ladr");
		int length_par = par_table.size();
		for (int i = 0; i < length_par; i++)
		{
			fprintf(file_var,"%17s\t%5s\t%5d\t%10s\t%5d\t%5d\n", par_table[i].vname.c_str(), par_table[i].vproc.c_str(), par_table[i].vkind, par_table[i].vtype.c_str(), par_table[i].vlev, par_table[i].vadr);
		}
		int length_pro= pro_table.size();
		for (int i = 0; i < length_pro; i++)
		{
			fprintf(file_pro,"%17s\t%10s\t%5d\t%5d\t%5d\n", pro_table[i].name.c_str(), pro_table[i].ptype.c_str(), pro_table[i].plev, pro_table[i].fadr, pro_table[i].ladr);
		}

	}

	// 分析入口
	void syntax_analize()
	{ // 程序->分程序
		cur_pro.push_back("main");//指代这是1级程序
		sub_P();
		advance();
		if (cur_word.flag == 25 && !error)
		{
			cout<<"语法分析成功\n"<<endl;
		}
		else
		{
			cout<<"语法分析失败\n"<<endl;
		}
	}
};

int main()
{
	const char *tex_err_filename = "C:/Users/18397/Desktop/code/src.err";
	const char *dyd_filename = "C:/Users/18397/Desktop/code/src.dyd";
	const char *dys_filename = "C:/Users/18397/Desktop/code/src.dys";
	const char *var_filename = "C:/Users/18397/Desktop/code/src.var";
	const char *pro_filename = "C:/Users/18397/Desktop/code/src.pro";
	const char *syntax_filename = "C:/Users/18397/Desktop/code/syntax.err";
	syntax_analysis syntax_entity(tex_err_filename, dyd_filename, dys_filename, var_filename, pro_filename, syntax_filename);
	syntax_entity.solution();

	return 0;
}