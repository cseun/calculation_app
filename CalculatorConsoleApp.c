#include <stdio.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS

// ���α׷� ó�� ��û ����
typedef enum {
	STATUS_PROCESS,		 // ���� ���� ���� (���� ����)
	STATUS_FORCE_EXIT,	 // ���� ���� (����)
	STATUS_EXIT,		 // ���� ����
	STATUS_RESTART       // ���α׷� �����
} ProcessRequestStatus;

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, const char* context);
ProcessRequestStatus validateExpression(int *exprLen, char* expression);

// parseExpression
// �Լ� ����: ������� �޾Ƽ� �����ڿ� �ǿ����ڸ� ���� ������� ���� ������� �Լ�
//	- TODO ��ȣ��
ProcessRequestStatus parseExpression(int numbers[], char operators[], int* numberIdx, int* operatorIdx);

// �Լ� ����: �����ڿ� �ǿ����� �迭�� ��ȸ�Ͽ� ����ϰ� ������� ���ϴ� �Լ�
//	- ������� �����Ѵ�.
//	1) ��ȣ�� �ȿ� �ִ� ���� ���� ���� ��� ����
//		- ��ȣ�� ������ ��� ȣ��� ���ʺ��� ���
//	2) ���� ������ ��� ���� ����
//		- ������ ���� üũ (������ ���� �� 0���� ������ ��� �����߻�)
//		- �߸��� �Է��̿��� ��� ����� ��û ���¸� ��ȯ�Ѵ�.
//	3) ���� ���� ��� ���� 
int calculate(char* expression);

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

	int numberIdx;
	int operatorIdx;
	char* expressionPtr = NULL;

	int numberCount;
	int operatorCount;

	printf("== ���� ���α׷� ==\n");

	while (1)
	{
		printf("����� ������� �Է��Ͻÿ�(���Ṯ�� x Ȥ�� mX) ex) 1+2*3\n");
		printf("[ ���� ]");
		printf("- ������ ���깮�� +, -, *, /\n");
		printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
		printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n:");

		isFirst = TRUE;
		ch = 0;
		context = "";
		exprLen = 0;
		exprSize = 100;

		// ===== �Է°� �޸� �Ҵ� =====
		expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			executeProgramControl(STATUS_FORCE_EXIT,"�޸� �Ҵ� ����. ������ ���� �ʿ�\n");
		}
		dynamicArrays[0] = expression;

		// ===== �Է� ó�� =====
		while ((ch = getchar()) != '\n' && ch != EOF) {
			// ù ���� ����Ű ó��
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

		exprLen = strlen(expression);

		// ���� ������ �������� Ȯ�� �� ���͸�
		ProcessRequestStatus status = validateExpression(&exprLen, expression);
		if (status == STATUS_RESTART) continue;

		// ���� �� ������ �и�		
		numbers = (int*)malloc(sizeof(int) * exprLen);
		operators = (char*)malloc(sizeof(char) * exprLen);

		if (!numbers || !operators) {
			executeProgramControl(STATUS_RESTART, "�޸� �Ҵ� ����\n\n");
			continue;
		}
		dynamicArrays[1] = numbers;
		dynamicArrays[2] = operators;

		numberIdx = 0;
		operatorIdx = 0;
		expressionPtr = expression;
		while (*expressionPtr) { // ǥ������ �����Ҷ� ����
			char* endptr;
			int num = strtol(expressionPtr, &endptr, 10);
			
			// �����Ͱ� ����Ű�� ���� ������ ���
			if (isdigit((unsigned char)*expressionPtr)) {
				numbers[numberIdx] = num;
				expressionPtr = endptr;
		
				// ���� ���� ������ ���� �������� ���
				if (*expressionPtr && strchr(validOperators, *expressionPtr)) {
					operators[operatorIdx] = *expressionPtr;
					expressionPtr++;
					operatorIdx++;
				}
				numberIdx++;
			} 
			//else if ( TODO ��ȣ �߰� )
			else {
				break;
			}
		}

		numberCount = numberIdx;
		operatorCount = operatorIdx;

		if (numberCount < 2 || operatorCount < 1) {
			executeProgramControl(STATUS_RESTART, "��꿡 �ʿ��� ���� Ȥ�� �����ڰ� ����. ���Է� ���ּ���\n\n");
			continue;
		}

		// === ���� ��� �� ��� ===
		printf("���͸��� ����: %s\n", expression);

		// ��� ó��
		int i = 0;
		while (i < operatorCount) {
			if (operators[i] == '*' || operators[i] == '/') {
				int a = numbers[i];
				int b = numbers[i + 1];

				if (operators[i] == '/' && b == 0) {
					executeProgramControl(STATUS_RESTART, "0���� ���� �� �����ϴ�\n\n");
					continue;
				}

				// i �� i+1 �� ��� ������� i�� ����
				numbers[i] = (operators[i] == '*') ? a * b : a / b;

				// i+1 ���� �� shift 
				for (int j = i + 1; j < numberCount - 1; j++)
				{
					numbers[j] = numbers[j + 1];
				}

				// i ���� �� shift
				for (int j = i; j < operatorCount - 1; j++)
				{
					operators[j] = operators[j + 1];
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
		int result = numbers[0];
		for (int i = 0; i < operatorCount; i++) {
			if (operators[i] == '+') {
				result += numbers[i + 1];
			}
			else {
				result -= numbers[i + 1];
			}
		}

		printf("��� ���: %d \n\n", result);

	}

	return 1;
}

// �Լ� ����: ���α׷� ó�� ��û ����(ProcessRequestStatus)�� �޾� �� ���¿� ���� ���α׷� ���� Ȥ�� �� ����.
//	- ���α׷� ������� �Լ� ������ ó���� �ȵǹǷ� ����� �� ��ȯ
//	- ���α׷� ó�� ��û ����(����/�����)��, ��¹��� �ް� ó���Ѵ�.
//		- ����: ���� �޸� ���� �� null �ʱ�ȭ�� ���� �޸� ���� ����	
//		- ���α׷� �����: 
//			1) ����� ���� ���
//			2) ���α׷� ����� ���� �� ProcessRequestStatus::STATUS_RESTART ��ȯ
//		- ���α׷� ������ ����
//			1) ���� ���� ���
//			2) ���� ������ (sleep(2000))
//			3) ���� exit(1);
//		- ���α׷� ���� ����
//			1) ���� ���� ���
//			2) ���� ������ (sleep(2000))
//			3) ���� exit(0);

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, const char* context)
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
			return executeProgramControl(STATUS_RESTART,
				"�ݴ� ��ȣ ')'�� �ʹ� �����ϴ�. ������ �ٽ� �Է����ּ���.\n\n");
		}
	}

	if (balance != 0) {
		return executeProgramControl(STATUS_RESTART, "��ȣ ¦�� ���� �ʽ��ϴ�. ������ �ٽ� �Է����ּ���.\n\n");
	}

	return STATUS_PROCESS;

}

ProcessRequestStatus parseExpression(int numbers[], char operators[], int* numberIdx, int* operatorIdx)
{

}
int calculate(char* expression) 
{

}