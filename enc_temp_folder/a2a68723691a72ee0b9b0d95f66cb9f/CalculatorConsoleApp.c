#include <stdio.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS

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

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context);
ProcessRequestStatus validateExpression(int *exprLen, char* expression);
ProcessRequestStatus parseExpression(ExpressionData* data);
int calculate(char* expression);

// �Լ� ����: �����ڿ� �ǿ����� �迭�� ��ȸ�Ͽ� ����ϰ� ������� ���ϴ� �Լ�
//	- ������� �����Ѵ�.
//	1) ��ȣ�� �ȿ� �ִ� ���� ���� ���� ��� ����
//		- ��ȣ�� ������ ��� ȣ��� ���ʺ��� ���
//	2) ���� ������ ��� ���� ����
//		- ������ ���� üũ (������ ���� �� 0���� ������ ��� �����߻�)
//		- �߸��� �Է��̿��� ��� ����� ��û ���¸� ��ȯ�Ѵ�.
//	3) ���� ���� ��� ���� 


void* dynamicArrays[3] = { NULL, NULL, NULL };
const char* validOperators = "*/+-()";
const char* exits = "xX";

int main()
{
	boolean isFirst;
	int ch;	
	char* context;
	size_t exprLen;
	size_t exprSize;
	char* expression;
	int* numbers;
	char* operators;
	int numberCount;
	int operatorCount;
	ProcessRequestStatus status;

	printf("== ���� ���α׷� ==\n");

	while (1)
	{
		isFirst = TRUE;
		ch = 0;
		context = "";
		exprLen = 0;
		exprSize = 100;

		printf("����� ������� �Է��Ͻÿ�(���Ṯ�� x Ȥ�� mX) ex) 1+2*3\n");
		printf("[ ���� ]\n");
		printf("- ������ ���깮�� +, -, *, /\n");
		printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
		printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n:");

		expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			executeProgramControl(STATUS_FORCE_EXIT,"�޸� �Ҵ� ����. ������ ���� �ʿ�\n");
		}
		dynamicArrays[0] = expression;

		// == ����� �Է� ==
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
		if (status == STATUS_RESTART) continue;

		// == ������, �ǿ����� �迭 �и� ==
		numbers = (int*)malloc(sizeof(int) * exprLen);
		operators = (char*)malloc(sizeof(char) * exprLen);
		if (!numbers || !operators) {
			executeProgramControl(STATUS_RESTART, "�޸� �Ҵ� ����\n\n");
			continue;
		}
		dynamicArrays[1] = numbers;
		dynamicArrays[2] = operators;

		numberCount = 0;
		operatorCount = 0;
		ExpressionData data = {
			expression,
			numbers,   
			numberCount, 
			operators,
			operatorCount 
		};

		status = parseExpression(&data);
		if (status == STATUS_RESTART) continue;

		// === ���� ��� �� ��� ===
		printf("���͸��� ����: %s\n", expression);

		numberCount = data.numberLen;
		operatorCount = data.operatorLen;
		
		int i = 0;
		while (i < operatorCount) {
			if (data.operators[i] == '*' || data.operators[i] == '/') {
				int a = data.numbers[i];
				int b = data.numbers[i + 1];

				if (data.operators[i] == '/' && b == 0) {
					executeProgramControl(STATUS_RESTART, "0���� ���� �� �����ϴ�\n\n");
					continue;
				}

				// i �� i+1�� ��� ����� i�� ����
				data.numbers[i] = (data.operators[i] == '*') ? a * b : a / b;

				// i+1 ���� �½���Ʈ
				for (int j = i + 1; j < numberCount - 1; j++) {
					data.numbers[j] = data.numbers[j + 1];
				}

				// ������ �½���Ʈ
				for (int j = i; j < operatorCount - 1; j++) {
					data.operators[j] = data.operators[j + 1];
				}

				// ���� �������� �ݺ��ǵ��� ����
				numberCount--;
				operatorCount--;
			}
			else {
				// ����, �������� �ƴ� ��� ���� ������ ��ȸ
				i++;
			}
		}

		// ����, ���� ���
		int result = data.numbers[0];
		for(int i = 0; i < operatorCount; i++) {
			if (data.operators[i] == '+') {
				result += data.numbers[i + 1];
			}
			else {
				result -= data.numbers[i + 1];
			}
		}

		printf("��� ���: %d \n\n", result);

	}

	return 1;
}

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context)
{
	if (context)
	{
		printf("%s\n", context);
	}

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
		if (strchr(validOperators, expression[i - 1]) && strchr(validOperators, expression[i])) {
			return executeProgramControl(STATUS_RESTART, "���� ���� �� ��Ÿ�� ���� ������ ����. ���Է� ��û\n\n");
		}
	}

	// ��ȣ �˻�
	int balance = 0;
	for (int i = 0; i < *exprLen; ++i) {
		if (expression[i] == '(')
			balance++;
		else if (expression[i] == ')')
			balance--;

		// �ݴ� ��ȣ�� �� ������ �ٷ� ����
		if (balance < 0) {
			return executeProgramControl(STATUS_RESTART, "�ݴ� ��ȣ ')'�� �ʹ� �����ϴ�. ������ �ٽ� �Է����ּ���.\n\n");
		}
	}

	if (balance != 0) {
		return executeProgramControl(STATUS_RESTART, "��ȣ ¦�� ���� �ʽ��ϴ�. ������ �ٽ� �Է����ּ���.\n\n");
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
		// ��ȣ�� ��� �߰�
		else if (*expressionPtr == '(' || *expressionPtr == ')') {
			data->operators[data->operatorLen++] = *expressionPtr;
			expressionPtr++;
		}
		else
		{
			return executeProgramControl(STATUS_RESTART, "���� �Ǵ� �߸��� ���� �߰�.\n\n");
		}
	}

	if (data->numberLen < 2 || data->operatorLen < 1) {
		return executeProgramControl(STATUS_RESTART, "��꿡 �ʿ��� ���� Ȥ�� �����ڰ� ����. ���Է� ���ּ���\n\n");
	}
	
	data->operators[data->operatorLen] = '\0';
	return STATUS_PROCESS;
}

int calculate(char* expression) 
{

}