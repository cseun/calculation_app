#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS

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
void* dynamicArrays[3] = { NULL, NULL, NULL };	// ���� �迭 �޸� �ѹ��� ����
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
		printf("����� ������� �Է��Ͻÿ�(���Ṯ�� x Ȥ�� mX) ex) 1+2*3\n");
		printf("[ ���� ]\n");
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
		printf("���͸��� ����: %s\n", expression);
		int result = 0;
		status = calculate(&data, &result);
		if (status == STATUS_RESTART) {
			continue;
		}

		printf("��� ���: %d\n\n", result);

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

ProcessRequestStatus calculate(ExpressionData* data, int* result)
{
	int i = 0;
	// ��ȣ ���
	while (i < data->operatorLen) {
		if (data->operators[i] == '(') {
			int depth = 1;
			int j = i + 1;

			// ¦�� �´� ')' ã��
			while (depth > 0) {
				if (data->operators[j] == '(') {
					depth++;
				}
				else if (data->operators[j] == ')') {
					depth--;
				}
				j++;
			}

			// i~j-1 ������ ��ȣ ����
			ExpressionData inner = {
				.expression = NULL,
				.numbers = &data->numbers[i],
				.numberLen = 0,
				.operators = &data->operators[i + 1],
				.operatorLen = 0
			};

			int innerResult = 0;
			ProcessRequestStatus status = calculate(&inner, &innerResult);
			if (status != STATUS_PROCESS)
				return status;

			// ����� numbers[i]�� �����
			data->numbers[i] = innerResult;

			// ��ȣ ���� ���� �� ����Ʈ
			int removeOps = (j - i);
			for (int k = i + 1; k < data->numberLen - removeOps + 1; k++)
			{
				data->numbers[k] = data->numbers[k + removeOps - 1];
			}
			for (int k = i; k < data->operatorLen - removeOps + 1; k++)
			{
				data->operators[k] = data->operators[k + removeOps];
			}

			data->numberLen -= (removeOps - 1);
			data->operatorLen -= removeOps;
			continue; // ���� ��ġ �ٽ� �˻� (��ø ��ȣ ����)
		}
		i++;

	}

	//���� ������
	i = 0;
	while (i < data->operatorLen) {
		if (data->operators[i] == '*' || data->operators[i] == '/') {
			int a = data->numbers[i];
			int b = data->numbers[i + 1];

			if (data->operators[i] == '/' && b == 0) {
				executeProgramControl(STATUS_RESTART, "0���� ���� �� �����ϴ�\n\n");
				return STATUS_RESTART;
			}

			data->numbers[i] = (data->operators[i] == '*') ? a * b : a / b;

			for (int j = i + 1; j < data->numberLen - 1; j++) {
				data->numbers[j] = data->numbers[j + 1];
			}
			for (int j = i; j < data->operatorLen - 1; j++) {
				data->operators[j] = data->operators[j + 1];
			}
			data->numberLen--;
			data->operatorLen--;
		}
		else {
			i++; // ����, �������� �ƴ� ��� ���� ������ ��ȸ
		}
	}

	// ����, ���� ���
	int temp = data->numbers[0];
	for (i = 0; i < data->operatorLen; i++) {
		if (data->operators[i] == '+') {
			temp += data->numbers[i + 1];
		}
		else {
			temp -= data->numbers[i + 1];
		}
	}

	*result = temp;
	return STATUS_PROCESS;

}