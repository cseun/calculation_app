#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <Windows.h>
#include <math.h>

// �Լ� ����
bool insertExpression(const char* validOperators, const char* exits, char** expressionPtr, int* exprSizePtr);	// ���� �Է�
bool validateExpression(const char* expression);	// ���� ���͸�
bool normalizeExpression(char* expression, int* exprSizePtr); // ����� ����
bool getCalculateResult(char* expression, double* result); // ���
double calculate(const char* expression);
int precedence(char op);
double applyOp(double a, double b, char op);

int main()
{
	const char* validOperators = "*/+-().";
	const char* exits = "xX";
	int exprSize = 100;
	char* expression = (char*)malloc(sizeof(char) * exprSize);
	if (!expression) {
		printf("���� �迭(expression) �޸� �Ҵ� ���� �߻�.\n\n");
		exit(0);
	}

	printf("== (�ܢ�������) ���� ���α׷� ==\n");
	printf("[ ���� ]\n");
	printf("����� ������� �Է��Ͻÿ�. (���Ṯ�� x Ȥ�� X)\n");
	printf("- ������ ���깮�� +, -, *, /, ��ȣ(, )\n");
	printf("- �Ҽ��� �ڸ��� 2�ڸ����� ǥ�õ˴ϴ�.\n");
	printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
	printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n\n");
	while (1)
	{
		printf("[ ���� �Է� ]\n:");
		//expression ���ΰ� �ʱ�ȭ

		if (!insertExpression(validOperators, exits, &expression, &exprSize)) continue;

		printf("[ ���� ���� ]\n");
		printf("�Էµ� ����(��/�� ��Ÿ ����): %s\n", expression);

		if (!validateExpression(expression)) continue;
		printf("- ���͸��� ����: %s\n", expression);

		if (!normalizeExpression(expression, &exprSize)) continue;
		printf("- ������ ����: %s\n", expression);

		double* result = (double*)malloc(sizeof(double) * exprSize);
		if (!result) {
			printf("���� ���(result) �޸� �Ҵ� ���� �߻�.\n\n");
			continue;
		}
		if (!getCalculateResult(expression, result)) continue;
		
		printf("��� ���: %.2f\n\n", *result);

		fflush(stdout);
	}

	return 1;
}

// ���� �Է� (���ʿ��� ��/�� ���� ���� ����)
bool insertExpression(const char* validOperators, const char* exits, char** expressionPtr, int* exprSizePtr)
{
	char* expression = *expressionPtr;
	int exprSize = *exprSizePtr;
	int exprIdx = 0;
	int ch;
	bool isFirst = true;
	int lastDigit = -1;
	int lastOp = -1;

	while ((ch = getchar()) != '\n' && ch != EOF) {
		if (isFirst) {
			if (strchr(exits, ch)) {
				printf("���α׷��� 2�� �� ����˴ϴ�.");
				Sleep(2000);
				exit(0);
			}
			isFirst = false;
		}

		// ù ���� �Ұ� ������ ����
		if (exprIdx == 0 && strchr("+*/)", ch)) continue;

		if (isdigit(ch) || strchr(validOperators, ch)) {
			// �޸� ���Ҵ�
			if (exprIdx + 1 >= exprSize) {
				exprSize *= 2;
				char* temp = (char*)realloc(expression, sizeof(char) * exprSize);
				if (!temp) {
					printf("���� �迭(expression) �޸� ���Ҵ� ���� �߻�.\n\n");
					return false;
				}
				expression = temp;
				*expressionPtr = expression;
				*exprSizePtr = exprSize;
			}

			expression[exprIdx] = (char)ch;

			// ������ ����/������ ��ġ ����
			if (isdigit(ch)) lastDigit = exprIdx;
			if (strchr(validOperators, ch)) lastOp = exprIdx;

			exprIdx++;
		}
	}
	expression[exprIdx] = '\0';

	// ������ ���� ���� ���ʿ��� ������ ����
	if (lastDigit != -1 && lastOp > lastDigit) {
		for (int i = lastDigit + 1; i < exprIdx; i++) {
			if (expression[i] != ')') {
				memmove(&expression[i], &expression[i + 1], strlen(expression + i));
				i--;
				exprIdx--;
			}
		}
		expression[exprIdx] = '\0';
	}

	if (ch == EOF && ferror(stdin)) {
		clearerr(stdin);
		printf("�Է� �� ���� �߻�.\n\n");
		return false;
	}

	if (exprIdx == 0) {
		printf("�Էµ� ������ ����.\n\n");
		return false;
	}

	return true;
}

// ���� ��ȿ�� �˻�
bool validateExpression(const char* expression) {
	int depth = 0;          // ��ȣ ����
	bool hasDot = false;    // ���� ���� �� �Ҽ��� ���� ����
	char prev = 0;          // ���� ���� ����

	for (int i = 0; expression[i] != '\0'; i++) {
		char ch = expression[i];

		// ��ȣ ¦ �˻�
		if (ch == '(') depth++;
		else if (ch == ')') depth--;

		if (depth < 0) return false; // ���� ��ȣ �ʰ�

		// ���� ������ �˻� (����: +-, --, *-, /-)
		if (prev != '\0' && ch != '\0' && strchr("+-*/", prev) && strchr("+-*/", ch)) {
			if (!(ch == '-' && strchr("+-*/", prev))) {
				printf("������ ���� ���� \n\n");
				return false;
			}
		}

		// �Ҽ��� �˻�
		if (ch == '.') {
			if (hasDot || !isdigit(prev)) return false;
			hasDot = true;
		}
		else if (!isdigit(ch)) {
			hasDot = false; // �Ҽ��� ���� ���� ����
		}

		prev = ch;
	}

	if (depth != 0) {
		printf("��ȣ ¦ ����ġ.\n\n");
		return false;
	}

	return true;
}

// ����� ���� (��ȣ, ���� ����/���� ��)
bool normalizeExpression(char* expression, int* exprSizePtr) {
	int exprSize = *exprSizePtr;
	int exprLen = strlen(expression);
	
	char* temp = (char*)malloc(sizeof(char) * (exprSize * 2));
	if (!temp) {
		printf("������ �ӽ� ������ �迭(temp) �޸� �Ҵ� ����.\n\n");
		return false;
	}

	int j = 0;
	// ��ȣ �� ���� �� ���� ����
	for (int i = 0; i < exprLen; i++) {
		char ch = expression[i];
		char next = (i + 1 < exprLen) ? expression[i + 1] : '\0';

		// 0) �� ��ȣ ����
		if (ch == '(' && next == ')') {
			i++; // ���ȣ �ǳʶٱ�
			continue;
		}

		// 1) ���� �ٷ� �� '(' �� * �߰�
		if (isdigit((unsigned char)ch) && next == '(') {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 2) ')' �ٷ� �� ���� �� * �߰�
		else if (ch == ')' && isdigit((unsigned char)next)) {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 3) ')' �ٷ� �� '(' �� * �߰�
		else if (ch == ')' && next == '(') {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 4) '-(' ���� ���� ��ȣ ó�� �� -1*(...)
		else if (ch == '-' && next == '(') {
			temp[j++] = '-';
			temp[j++] = '1';
			temp[j++] = '*';
		}
		else {
			temp[j++] = ch;
		}

		// �޸� ���Ҵ�
		if (j + 1 > exprSize) {
			exprSize *= 2;
			char* temptemp = (char*)realloc(temp, sizeof(char) * exprSize);
			if (!temptemp) {
				printf("���� �迭(expression) �޸� ���Ҵ� ���� �߻�.\n\n");
				free(temp);
				return false;
			}
			temp = temptemp;
		}
	}
	temp[j] = '\0';
	int maxCopySize = *exprSizePtr - 1;
	strncpy(expression, temp, maxCopySize);
	expression[maxCopySize] = '\0';

	exprLen = strlen(expression);
	j = 0;

	// ���� ��ȣ ����
	for (int i = 0; i < exprLen; i++) {
		char ch = expression[i];
		if (ch == '(') {
			int innerStart = i + 1;
			int innerEnd = innerStart;

			// ��ȣ ���� ����
			if (expression[innerStart] == '-' || expression[innerStart] == '+') innerEnd++;
			// ���� �� �Ҽ���
			while (isdigit(expression[innerEnd]) || expression[innerEnd] == '.') innerEnd++;
			// ���� ��ȣ�̸� ���� ���븸 ����
			if (expression[innerEnd] == ')' && innerEnd > innerStart) {
				for (int k = innerStart; k < innerEnd; k++)
				{
					temp[j++] = expression[k];
				}
				i = innerEnd;
				continue;
			}
		}
		temp[j++] = ch;
	}
	temp[j] = '\0';

	strncpy(expression, temp, maxCopySize);
	expression[maxCopySize] = '\0';

	*exprSizePtr = (int)strlen(expression) + 1;
	free(temp);

	return true;
}

// ���
bool getCalculateResult(char* expression, double* finalResult)
{
	char* startPtr = NULL;
	char* endPtr = NULL;
	int exprLen = strlen(expression);
	double result = 0.0;

	char* innerExpression = (char*)malloc(sizeof(char) * (exprLen + 1));
	if (!innerExpression) {
		printf("��ȣ ���� ���� �ӽ����� �迭(innerExpression) �޸� ���Ҵ� ���� �߻�.\n\n");
		return false;
	}
	char* tempExpression = (char*)malloc(sizeof(char) * (exprLen * 2 + 1));
	if (!tempExpression) {
		printf("���� �ӽ����� �迭(tempExpression) �޸� ���Ҵ� ���� �߻�.\n\n");
		return false;
	}

	strcpy(tempExpression, expression);
	while ((startPtr = strrchr(expression, '(')) != NULL)
	{
		// ���� ��ȣ�� ������ �������� �迭�� ��� ��� �Լ��� ������. 
		endPtr = strchr(startPtr, ')');
		if (!endPtr) {
			printf("���� ��ȣ�� ����.\n\n");
			return false;
		}

		// ���� ���� ����
		int innerLen = endPtr - startPtr - 1;
		strncpy(innerExpression, startPtr + 1, innerLen);
		innerExpression[innerLen] = '\0';

		result = calculate(innerExpression);
		

		// �ӽ� ���� �迭�� ���� ��ȣ ��, ��� ���, ������ȣ �� ���ڵ��� ���� �����Ѵ�. 
		sprintf(tempExpression, "%.*s%.2f%s",
			(int)(startPtr - expression),
			expression,
			result,
			endPtr + 1
		);
		strcpy(expression, tempExpression);
		printf("%s", tempExpression);
	}

	// ��ȣ�� ���ٸ� ��ü�� �迭�� ��� ��� �Լ��� ������.
	result = calculate(tempExpression);

	*finalResult = round(result * 100.0) / 100.0;;
	free(innerExpression);
	free(tempExpression);
	
	return true;
}

double calculate(const char* expression)
{
	int exprLen = strlen(expression);
	double* numbers = (double*)malloc(sizeof(double) * (exprLen + 1));
	char* operators = (char*)malloc(sizeof(char) * (exprLen + 1));
	if (!numbers || !operators) {
		return false;
	}

	int numTop = -1;
	int opTop = -1;
	int i = 0;
	bool isNegative = false;
	
	while (expression[i] != '\0') {
		char ch = expression[i];

		// ���� ��ȣ ó�� (ó���̰ų� ������ �������� ��)
		if (ch == '-' && (i == 0 || strchr("+-*/", expression[i - 1]))) {
			isNegative = true;
			i++;
			continue;
		}

		// ���� ó��
		if (isdigit((unsigned char)ch)) {
			bool isFraction = false;
			double val = 0;
			double divisor = 1.0;
			while (isdigit((unsigned char)expression[i]) || expression[i] == '.')
			{
				if (expression[i] == '.') {
					isFraction = true;
					i++;
					continue;
				}
				
				val = val * 10 + (expression[i] - '0');
				if (isFraction) divisor *= 10.0;
				
				i++;
			}

			val /= divisor;

			if (isNegative) {
				val = -val;
				isNegative = false;
			}

			numbers[++numTop] = val;
			continue;
		}

		// ������ ó��
		if (strchr("+-*/", ch)) {
			while (opTop >= 0 && precedence(operators[opTop]) >= precedence(ch)) {
				double b = numbers[numTop--];
				double a = numbers[numTop--];
				char op = operators[opTop--];
				numbers[++numTop] = applyOp(a, b, op);
			}
			operators[++opTop] = ch;
		}

		i++;
	}

	// ���� ���� ó��
	while (opTop >= 0) {
		double b = numbers[numTop--];
		double a = numbers[numTop--];
		char op = operators[opTop--];
		numbers[++numTop] = applyOp(a, b, op);
	}

	double result = numbers[numTop];
	free(numbers);
	free(operators);

	return result;
}

double applyOp(double a, double b, char op) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	case '/': return b != 0 ? a / b : 0;
	default: return 0;
	}
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}