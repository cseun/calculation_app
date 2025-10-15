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

const char* validOperators = "*/+-"; // TODO ( �� ) �߰� �ϱ�
const char* exits = "xX";

// executeProgramControl
// �Լ� ����: ���α׷� ó�� ��û ����(ProcessRequestStatus)�� �޾� �� ���¿� ���� ���α׷� ���� Ȥ�� �� ����.
//	- ���α׷� ������� �Լ� ������ ó���� �ȵǹǷ� ����� �� ��ȯ
//	- ���α׷� ó�� ��û ����(����/�����)��, ��¹��� �ް� ó���Ѵ�.
//		- ����: ���� �޸� ������ ���� �޸� ���� ����	
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
ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context);

// validateExpression
// �Լ� ����: ����� �Է� ���� ���͸��ϸ�, ������ �߻��ϴ� ��� ó�� ��û ���¸� ��ȯ�Ѵ�.
// - �߸��� �Է°� ���͸�
//	1) �� ������ ����
//		- �����ڰ� ���� ��ȣ�� ��� ������
//	2) �� ������ ����
//		- �����ڰ� ���� ��ȣ�� ��� ������
//	3) ����� ���� �ִ��� üũ
//		- ����� ���� ���� ��� ����� ��û ���¸� ��ȯ�Ѵ�.
//	4) ������ ���� �Է� ���� üũ
//		- �߸��� �Է��̿��� ��� ����� ��û ���¸� ��ȯ�Ѵ�.
//  5) TODO ��ȣ�� ���� �̷���� ���� üũ
//		- ���� ��ħ (, ) ��ȣ�� ������ �־���Ѵ�.
//		- �߸��� �Է��̿��� ��� ����� ��û ���¸� ��ȯ�Ѵ�.
// - ���������� ó���Ǿ��� ��� ���� ���� ���� ���¸� ��ȯ�Ѵ�.
ProcessRequestStatus validateExpression(int* exprLen, char* expression);

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

int main()
{

	printf("== ���� ���α׷� ==\n");

	while (1)
	{
	HERE:
		printf("����� ������� �Է��Ͻÿ�(���Ṯ�� x Ȥ�� mX) ex) 1+2*3\n");
		printf("- ������ ���깮�� +, -, *, /\n");
		printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
		printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n:");

		boolean isFirst = TRUE;
		char* validOperators = "*/+-";
		char* exits = "xX";
		size_t exprLen = 0;
		size_t exprSize = 100;

		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			printf("�޸� �Ҵ� ����. ������ ���� �ʿ�\n");
			return 0;
		}

		// [4-2] === ����� �Է� �� ���� ===
		int ch = 0;
		while ((ch = getchar()) != '\n' && ch != EOF) {
			// ù ���� ����Ű ó��
			if (isFirst) {
				if (strchr(exits, ch)) {
					printf("���α׷� ����\n");
					Sleep(2000);
					return 1;
				}
				isFirst = FALSE;
			}

			if (isdigit(ch) || strchr(validOperators, ch)) {
				if (exprLen + 1 >= exprSize) {
					exprSize *= 2;
					char* temp = (char*)realloc(expression, sizeof(char) * exprSize);
					if (!temp) {
						printf("�޸� ���Ҵ� ����. ������ ���� �ʿ�\n");
						free(expression);
						goto HERE;
					}
					expression = temp;
				}
				expression[exprLen++] = (char)ch;
			}
		}
		expression[exprLen] = '\0';

		if (ch == EOF && ferror(stdin)) {
			printf("�Է� �� �о���� �� ���� �߻�. ���Է� ���ּ���.");
			clearerr(stdin);
			free(expression);
			continue;
		}

		if (exprLen == 0) {
			printf("����� ������ �����ϴ�. �ٽ� �Է����ּ���.\n\n");
			continue;
		}

		// [5��] === ����� ���� ���� ===
		exprLen = strlen(expression);
		// �� ������ ����
		while (exprLen > 0 && strchr(validOperators, expression[0])) {
			memmove(expression, expression + 1, strlen(expression) + 1);
			exprLen--;
		}

		// �� ������ ����
		while (exprLen > 0 && strchr(validOperators, expression[exprLen - 1])) {
			expression[--exprLen] = '\0';
		}

		// ����� ���� ���� ����
		if (exprLen == 0) {
			printf("���� ������ ������ �ùٸ��� �ʽ��ϴ�. �ٽ� �Է����ּ���.\n\n");
			continue; //�Է����� ���ư�
		}

		// ������ ���� ����
		for (int exprIdx = 1; exprIdx < exprLen; ++exprIdx) {
			if (strchr(validOperators, expression[exprIdx - 1]) && strchr(validOperators, expression[exprIdx])) {
				printf("���� ���� �� ��Ÿ�� ���� ������ ����. ���Է� ��û\n\n");
				goto HERE;
			}
		}

		// ���� �� ������ �и�		
		int* numbers = (int*)malloc(sizeof(int) * exprLen);
		char* operators = (char*)malloc(sizeof(char) * exprLen);

		if (!numbers || !operators) {
			printf("�޸� �Ҵ� ����\n");
			free(numbers);
			free(operators);
			goto HERE; // �Է����� ����
		}

		int numberIdx = 0;
		int operatorIdx = 0;
		char* expressionPtr = expression;
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

		int numberCount = numberIdx;
		int operatorCount = operatorIdx;

		if (numberCount < 2 || operatorCount < 1) {
			printf("��꿡 �ʿ��� ���� Ȥ�� �����ڰ� ����. ���Է� ���ּ���.\n\n");
			continue;
		}

		// [7��] === ���� ��� �� ��� ===
		printf("���͸��� ����: %s\n", expression);

		// ��� ó��
		int i = 0;
		while (i < operatorCount) {
			if (operators[i] == '*' || operators[i] == '/') {
				int a = numbers[i];
				int b = numbers[i + 1];

				if (operators[i] == '/' && b == 0) {
					printf("0���� ���� �� �����ϴ�\n\n");
					goto HERE; //�Է����� ���ư�
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
		for (int j = 0; j < operatorCount; j++) {
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

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context)
{

}
ProcessRequestStatus validateExpression(int* exprLen, char* expression)
{

}
ProcessRequestStatus parseExpression(int numbers[], char operators[], int* numberIdx, int* operatorIdx)
{

}
int calculate(char* expression) 
{

}