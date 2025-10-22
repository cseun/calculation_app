#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <Windows.h>

// �Լ� ����
bool insertExpression(const char* validOperators, const char* exits, void** dynamicMemory, char* expression, int exprSize);	// ���� �Է�
bool validateExpression(const char* expression);	// ���� ���͸�
bool normalizeExpression(char* expression, void** dynamicMemory, int exprLen); // ����� ����
bool getCalculateResult(char* expression, double* result); // ���
void resetProgram(void** dynamicMemorys, int dynamicMemoryCnt);
double calculate(const char* expression, int exprLen);
int precedence(char op);
double applyOp(double a, double b, char op);

int main()
{
	void* dynamicMemorys[4] = { NULL, NULL, NULL, NULL };	// ���� �迭 �޸� �ѹ��� ����
	int dynamicMemoryCnt = sizeof(dynamicMemorys) / sizeof(void*);
	const char* validOperators = "*/+-().";
	const char* exits = "xX";

	printf("== ���� ���α׷� ==\n");

	while (1)
	{
		resetProgram(dynamicMemorys, dynamicMemoryCnt);

		printf("(�ܢ�������) ���α׷� ���� \n");
		printf("[ ���� ]\n");
		printf("����� ������� �Է��Ͻÿ�. (���Ṯ�� x Ȥ�� X)\n");
		printf("- ������ ���깮�� +, -, *, /, ��ȣ(, )\n");
		printf("- �Ҽ��� �ڸ��� 2�ڸ����� ǥ�õ˴ϴ�.\n");
		printf("- ���� ���깮�� ���� ���ڴ� �����ϰ� ���˴ϴ�. \n");
		printf("- ù ���ڰ� ���Ṯ��(X, X)�� ��� ���α׷��� ����˴ϴ�. \n:");

		int dynamicMemoryIdx = 0, exprSize = 100;
		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			printf("���� �迭(expression) �޸� �Ҵ� ���� �߻�.\n\n");
			continue;
		}
		dynamicMemorys[dynamicMemoryIdx] = expression;

		if (!insertExpression(validOperators, exits, &dynamicMemorys[dynamicMemoryIdx], expression, exprSize)) {
			if (strchr(exits, expression[0])) {
				printf("���α׷��� 2�� �� ����˴ϴ�.");
				resetProgram(dynamicMemorys, dynamicMemoryCnt);
				Sleep(2000);
				exit(0);
			}
			continue;
		}
		dynamicMemoryIdx++;

		printf("[ ���� ���� ]\n");
		printf("�Էµ� ����(��/�� ��Ÿ ����): %s\n", expression);

		if (!validateExpression(expression)) continue;
		printf("- ���͸��� ����: %s\n", expression);

		if (!normalizeExpression(expression, &dynamicMemorys[dynamicMemoryIdx], (int)strlen(expression))) continue;
		printf("- ������ ����: %s\n", expression);
		dynamicMemoryIdx++;

		double result = 0.0;
		if (!getCalculateResult(expression, &result)) continue;
		printf("��� ���: %.2f\n\n", result);

		fflush(stdout);
	}

	return 0;
}

// ���� �Է� (���ʿ��� ��/�� ���� ���� ����)
bool insertExpression(const char* validOperators, const char* exits, void** dynamicMemory, char* expression, int exprSize)
{
	int exprIdx = 0;
	int ch;
	bool isFirst = true;
	int lastDigit = -1;
	int lastOp = -1;

	while ((ch = getchar()) != '\n' && ch != EOF) {
		if (isFirst) {
			if (strchr(exits, ch)) {
				expression[exprIdx] = ch;
				expression[exprIdx + 1] = '\0';
				return false;
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
				expression = temp; // realloc �� �ٽ� ����
				*dynamicMemory = temp;
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
			if (!(prev == '-' && strchr("*/+", ch))) {
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
bool normalizeExpression(char* expression, void** dynamicMemory, int exprLen) {
	int exprSize = exprLen + 1;
	char* temp = (char*)malloc(sizeof(char) * (exprSize * 2));
	if (!temp) {
		printf("������ �ӽ� ������ �迭(temp) �޸� �Ҵ� ����.\n\n");
		free(temp);
		return false;
	}

	int j = 0;
	int exprIdx = j;
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
		if (isdigit(ch) && next == '(') {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 2) ')' �ٷ� �� ���� �� * �߰�
		else if (ch == ')' && isdigit(next)) {
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
				return false;
			}
			temp = temptemp; // realloc �� �ٽ� ����
			*dynamicMemory = temptemp;
		}
	}
	temp[j] = '\0';
	strcpy(expression, temp);

	j = 0;
	exprLen = (int)strlen(expression);
	// ���� ��ȣ ����
	for (int i = 0; i < exprLen; i++) {
		char ch = expression[i];
		if (ch == '(') {
			int innerStart = i + 1;
			int innerEnd = innerStart;

			// ��ȣ ���� ����
			if (expression[innerStart] == '-' || expression[innerStart] == '+')
				innerEnd++;

			// ���� �� �Ҽ���
			while (isdigit(expression[innerEnd]) || expression[innerEnd] == '.')
				innerEnd++;

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
	strcpy(expression, temp);
	exprLen = (int)strlen(expression);

	free(temp);

	return true;
}

// ���
bool getCalculateResult(char* expression, double* finalResult)
{
	char* startPtr = NULL;
	char* endPtr = NULL;
	int exprLen = strlen(expression);
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
		// 2 - 1. ���� ��ȣ�� ������ �������� �迭�� ��� ��� �Լ��� ������. 
		endPtr = strchr(startPtr, ')');
		if (!endPtr) {
			printf("���� ��ȣ�� ����.\n\n");
			return false;
		}

		// ���� ���� ����
		int innerLen = endPtr - startPtr - 1;
		strncpy(innerExpression, startPtr + 1, innerLen);
		innerExpression[innerLen] = '\0';

		double result = calculate(innerExpression, strlen(innerExpression));

		// 3. �ӽ� ���� �迭�� ���� ��ȣ ��, ��� ���, ������ȣ �� ���ڵ��� ���� �����Ѵ�. 
		sprintf(tempExpression, "%.*s%.2f%s",
			(int)(startPtr - expression),
			expression,
			result,
			endPtr + 1
		);
		strcpy(expression, tempExpression);
	}

	// 2 - 2. ���� ��ȣ�� ���ٸ� ��ü�� �迭�� ��� ��� �Լ��� ������.
	double result = calculate(tempExpression, strlen(tempExpression));

	*finalResult = result;
	free(innerExpression);
	free(tempExpression);
	
	return true;
}

double calculate(const char* expression, int exprLen)
{
	double result = 0.0;
	double* numbers = (double*)malloc(sizeof(double) * exprLen);
	char* operators = (char*)malloc(sizeof(char) * exprLen);
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
		if (isdigit(ch)) {
			bool isFraction = false;
			double val = 0;
			double divisor = 1.0;
			while (isdigit(expression[i]) || expression[i] == '.') 
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

	return numbers[numTop];
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

void resetProgram(void** dynamicMemorys, int dynamicMemoryCnt)
{
	for (int i = 0; i < dynamicMemoryCnt; i++) {
		if (dynamicMemorys[i]) {
			free(dynamicMemorys[i]);
			dynamicMemorys[i] = NULL;
		}
	}
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}
