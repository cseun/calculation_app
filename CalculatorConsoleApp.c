#include <stdio.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS


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

		int input = 0;
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
		while ((input = getchar()) != '\n' && input != EOF) {
			// ù ���� ����Ű ó��
			if (isFirst) {
				if (strchr(exits, input)) {
					printf("���α׷� ����\n");
					Sleep(2000);
					return 1;
				}
				isFirst = FALSE;
			}

			if (isdigit(input) || strchr(validOperators, input)) {
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
				expression[exprLen++] = (char)input;
			}
		}

		if (input == EOF && ferror(stdin)) {
			printf("�Է� �� �о���� �� ���� �߻�. ���Է� ���ּ���.");
			clearerr(stdin);
			free(expression);
			continue;
		}

		expression[exprLen] = '\0';
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
		while (*expressionPtr) {
			if (isdigit((unsigned char)*expressionPtr)) {
				char* endptr;
				int num = strtol(expressionPtr, &endptr, 10);
				numbers[numberIdx++] = num;
				expressionPtr = endptr;

				operators[operatorIdx++] = *expressionPtr;
				expressionPtr++;
			}
		}

		if (numberIdx < 2 || operatorIdx < 1) {
			printf("��꿡 �ʿ��� ���� Ȥ�� �����ڰ� ����.\n\n");
			continue;
		}

		// ��� ó��
		int i = 0;
		while (i < operatorIdx) {
			if (operators[i] == '*' || operators[i] == '/') {
				int a = numbers[i];
				int b = numbers[i + 1];

				if (operators[i] == '/' && b == 0) {
					printf("0���� ���� �� �����ϴ�\n\n");
					goto HERE; //�Է����� ���ư�
				}
				numbers[i] = (operators[i] == '*') ? a * b : a / b;
				for (int j = i + 1; j < numberIdx - 1; j++)
				{
					numbers[j] = numbers[j + 1];
				}
				for (int j = i; j < operatorIdx - 1; j++)
				{
					operators[j] = operators[j + 1];
				}
				numberIdx--;
				operatorIdx--;
			}
			else {
				i++;
			}
		}

		printf("���͸��� ��� ����: %s\n", expression);
		int result = numbers[0];
		for (int j = 0; j < operatorIdx; j++) {
			if (operators[j] == '+') result += numbers[j + 1];
			else result -= numbers[j + 1];
		}

		printf("��� ���: %d \n\n", result);

	}

	return 1;
}