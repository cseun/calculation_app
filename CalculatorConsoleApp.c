#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS
//123 + 2 * 2 + (3 + 2) -> 129 || (3+2) -> 3
// enum, struct, static ����
typedef enum {
	STATUS_PROCESS,		 // ���� ���� ���� (���� ����)
	STATUS_FORCE_EXIT,	 // ���� ���� (����)
	STATUS_EXIT,		 // ���� ����
	STATUS_RESTART       // ���α׷� �����
} ProcessRequestStatus;
typedef struct {
	const char* expression; // ���� ����
	int* numbers;           // �ǿ����� �迭
	int  numberLen;         // �ǿ����� ����
	char* operators;        // ������ �迭
	int  operatorLen;       // ������ ����
} ExpressionData;
void* dynamicArrays[3] = { NULL, 
NULL, NULL };	// ���� �迭 �޸� �ѹ��� ����
const char* validOperators = "*/+-()";
const char* validBasicOperators = "*/+-";
const char* exits = "xX";

// �Լ� ����
ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context);
ProcessRequestStatus validateExpression(int *exprLen, char* expression);
ProcessRequestStatus parseExpression(ExpressionData* data);
ProcessRequestStatus calculate(ExpressionData* data, int* result);

int main()
{
	ProcessRequestStatus status;

	printf("== ���� ���α׷� ==\n");

	while (1)
	{
		printf("[ ���� ]\n");
		printf("����� ������� �Է��Ͻÿ�(���Ṯ�� x Ȥ�� mX) ex) 1+2*3\n");
		printf("- ������ ���깮�� +, -, *, /\n");
		printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
		printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n:");
		
		int exprLen = 0, exprSize = 100;
		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			executeProgramControl(STATUS_FORCE_EXIT,"�޸� �Ҵ� ����. ������ ���� �ʿ�\n");
		}

		dynamicArrays[0] = expression;

		// == ����� �Է� ==
		bool isFirst = TRUE;
		int ch;
		while ((ch = getchar()) != '\n' && ch != EOF) {
			if (isFirst) {
				if (strchr(exits, ch)) {
					executeProgramControl(STATUS_EXIT, "����� ���� ��û");
				}
				isFirst = FALSE;
			}

			if (isdigit(ch) || strchr(validOperators, ch)) {
				if (exprLen + 1 >= exprSize) {
					exprSize *= 2;
					char* temp = (char*)realloc(expression, sizeof(char) * exprSize);
					if (!temp) {
						executeProgramControl(STATUS_FORCE_EXIT, "�޸� ���Ҵ� ����. ������ ���� �ʿ�\n");
					}
					expression = temp;
					dynamicArrays[0] = expression; // realloc �� �ٽ� ����
				}
				expression[exprLen++] = (char)ch;
			}
		}
		expression[exprLen] = '\0';

		if (ch == EOF && ferror(stdin)) {
			clearerr(stdin);
			executeProgramControl(STATUS_RESTART, "�Է� �� �о���� �� ���� �߻�.���Է� ���ּ���.\n");
			continue;
		}
		if (exprLen == 0) {
			executeProgramControl(STATUS_RESTART, "����� ������ �����ϴ�. �ٽ� �Է����ּ���.\n\n");
			continue;
		}

		// == ���� ���͸� ==
		exprLen = strlen(expression);
		status = validateExpression(&exprLen, expression);
		if (status == STATUS_RESTART) {
			continue;
		}

		// == ������, �ǿ����� �迭 �и� ==
		int* numbers = (int*)malloc(sizeof(int) * exprLen);
		char* operators = (char*)malloc(sizeof(char) * exprLen);
		if (!numbers || !operators) {
			executeProgramControl(STATUS_RESTART, "�޸� �Ҵ� ����\n\n");
			continue;
		}
		dynamicArrays[1] = numbers;
		dynamicArrays[2] = operators;

		ExpressionData data = {
			expression,
			numbers,   
			0,  // numberLen
			operators,
			0  // operatorLen
		};
		status = parseExpression(&data);

		if (status == STATUS_RESTART) {
			continue;
		}

		// === ���� ��� �� ��� ===
		printf("\n[��� ���]\n");
		printf("- ���͸��� ����: %s\n", expression);
		printf("-- ������:");
        for (int i = 0; i < data.operatorLen; i++) printf("%c ", operators[i]);
        printf("\n-- �ǿ�����:");
        for (int i = 0; i < data.numberLen; i++) printf("%d ", numbers[i]);

		int result = 0;
		status = calculate(&data, &result);
		if (status == STATUS_RESTART) {
			continue;
		}

		printf("\n==> ��� ���: %d\n\n", result);
		executeProgramControl(STATUS_RESTART, "");
	}

	return 1;
}

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context)
{
	if (context)
		printf("%s\n", context);

	// ���� �޸� ���� (TODO ���� ���������� ó��)
	for (int i = 0; i < 3; i++) {
		if (dynamicArrays[i]) {
			free(dynamicArrays[i]);
			dynamicArrays[i] = NULL;
		}
	}

	switch (status)
	{
		case STATUS_FORCE_EXIT:
			// ���� ���� (����)
			printf("���α׷��� 2�� �� ���� ����˴ϴ�.");
			Sleep(2000);
			exit(1);
			break;
		
		case STATUS_EXIT:
			// ���� ����
			printf("���α׷��� 2�� �� ���� ����˴ϴ�.");
			Sleep(2000);
			exit(0);
			break;

		case STATUS_RESTART:
			// ���α׷� �����
			printf("=== ���α׷� ����� ===\n");
			return STATUS_RESTART;

		default: 
			return STATUS_PROCESS;
	}
}

ProcessRequestStatus validateExpression(int* exprLen, char* expression)
{
	// �� ������ ����
	while (*exprLen > 0 && strchr(validOperators, expression[0])) {
		if (expression[0] == '(') break;
		memmove(expression, expression + 1, strlen(expression));
		(*exprLen)--;
	}

	// �� ������ ����
	while (*exprLen > 0 && strchr(validOperators, expression[*exprLen - 1])) {
		if (expression[*exprLen - 1] == ')') break;
		expression[--(*exprLen)] = '\0';
	}

	// ����� ���� ���� ����
	if (*exprLen == 0) {
		return executeProgramControl(STATUS_RESTART, "���� ���� �� ������ ��� �ֽ��ϴ�. �ٽ� �Է����ּ���.\n\n");
	}

	// ������ ���� ���� �˻�
	for (int i = 1; i < *exprLen; ++i) {
		if (strchr(validBasicOperators, expression[i - 1]) && strchr(validBasicOperators, expression[i])) {
			return executeProgramControl(STATUS_RESTART, "���� ���� �� ��Ÿ�� ���� ������ ����. ���Է� ��û\n\n");
		}
	}

	// ��ȣ �˻�
	// 1. ( ��: ����, �� �ٸ� '(' , ���� +/-
	// 2. ) ��: �� �ٸ� ) �Ǵ� '(' �Ǵ� �⺻ ������
	// 3. ���� �ٷ� �ڿ� ���� ��ȣ: ���� ���� (��: 3(2+1)) _ �����߻����� ���� 

	int balance = 0;
	for (int i = 0; i < *exprLen; ++i) {
		if (expression[i] == '(')
			balance++;
		else if (expression[i] == ')')
			balance--;

			if (balance < 0) {
			return executeProgramControl(STATUS_RESTART, "�ݴ� ��ȣ ')'�� �ʹ� �����ϴ�. ������ �ٽ� �Է����ּ���.\n\n");
		}
	}

	if (balance != 0) {
		return executeProgramControl(STATUS_RESTART, "��ȣ ¦�� ���� �ʽ��ϴ�. �ٽ� �Է����ּ���.\n\n");
	}

	return STATUS_PROCESS;

}

ProcessRequestStatus parseExpression(ExpressionData* data)
{
	char* expressionPtr = data->expression;
	while (*expressionPtr) { // ǥ������ �����Ҷ� ����
		char* endptr;
		int num = strtol(expressionPtr, &endptr, 10);

		// �����Ͱ� ����Ű�� ���� ������ ���
		if (isdigit((unsigned char)*expressionPtr)) {
			data->numbers[data->numberLen++] = num;
			expressionPtr = endptr;

			// ���� �ڿ� �����ڰ� ������ �߰�
			if (*expressionPtr && strchr(validOperators, *expressionPtr)) {
				data->operators[data->operatorLen++] = *expressionPtr;
				expressionPtr++;
			}
		} 
		else if (strchr(validOperators, *expressionPtr)) {
			// ������ �Ǵ� ��ȣ
			data->operators[data->operatorLen++] = *expressionPtr;
			expressionPtr++;
		/*else if (*expressionPtr == '(' || *expressionPtr == ')') { 
			data->operators[data->operatorLen++] = *expressionPtr;
			expressionPtr++;*/
		} else {
			return executeProgramControl(STATUS_RESTART, "���� �Ǵ� �߸��� ���� �߰�.\n\n");
		}
	}

	if (data->numberLen < 2 || data->operatorLen < 1) {
		return executeProgramControl(STATUS_RESTART, "��꿡 �ʿ��� ���� Ȥ�� �����ڰ� ����. ���Է� ���ּ���\n\n");
	}
	
	data->operators[data->operatorLen] = '\0';
	return STATUS_PROCESS;
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

int applyOp(int a, int b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/': 
        if (b == 0) {
            printf("0���� ���� �� �����ϴ�.\n");
            return 0;
        }
        return a / b;
    }
    return 0;
}

ProcessRequestStatus calculate(ExpressionData* data, int* result)
{
	const char* s = data->expression;
	int values[256]; int vTop = -1;
	char ops[256]; int oTop = -1;

	for (int i = 0; s[i]; ) {
		if (isspace((unsigned char)s[i])) { i++; continue; }

		if (isdigit((unsigned char)s[i])) {
			int val = 0;
			while (isdigit((unsigned char)s[i])) {
				val = val * 10 + (s[i] - '0');
				i++;
			}
			values[++vTop] = val;
		}
		else if (s[i] == '(') {
			ops[++oTop] = s[i];
			i++;
		}
		else if (s[i] == ')') {
			while (oTop >= 0 && ops[oTop] != '(') {
				int b = values[vTop--];
				int a = values[vTop--];
				char op = ops[oTop--];
				values[++vTop] = applyOp(a, b, op);
			}
			oTop--; // '(' ����
			i++;
		}
		else if (strchr("+-*/", s[i])) {
			while (oTop >= 0 && precedence(ops[oTop]) >= precedence(s[i])) {
				int b = values[vTop--];
				int a = values[vTop--];
				char op = ops[oTop--];
				values[++vTop] = applyOp(a, b, op);
			}
			ops[++oTop] = s[i];
			i++;
		}
		else {
			i++; // ����
		}
	}

	while (oTop >= 0) {
		int b = values[vTop--];
		int a = values[vTop--];
		char op = ops[oTop--];
		values[++vTop] = applyOp(a, b, op);
	}

	*result = values[vTop];
	return STATUS_PROCESS;
}