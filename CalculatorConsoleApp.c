#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <Windows.h>

// 함수 선언
bool insertExpression(const char* validOperators, const char* exits, void** dynamicMemory, char* expression, int exprSize);	// 계산식 입력
bool validateExpression(const char* expression);	// 계산식 필터링
bool normalizeExpression(char* expression, void** dynamicMemory, int exprLen); // 연산식 보정
bool getCalculateResult(char* expression, double* result); // 계산
void resetProgram(void** dynamicMemorys, int dynamicMemoryCnt);
double calculate(const char* expression, int exprLen);
int precedence(char op);
double applyOp(double a, double b, char op);

int main()
{
	void* dynamicMemorys[4] = { NULL, NULL, NULL, NULL };	// 동적 배열 메모리 한번에 관리
	int dynamicMemoryCnt = sizeof(dynamicMemorys) / sizeof(void*);
	const char* validOperators = "*/+-().";
	const char* exits = "xX";

	printf("== 계산기 프로그램 ==\n");

	while (1)
	{
		resetProgram(dynamicMemorys, dynamicMemoryCnt);

		printf("(●ˇ∀ˇ●) 프로그램 시작 \n");
		printf("[ 설명 ]\n");
		printf("계산할 연산식을 입력하시오. (종료문자 x 혹은 X)\n");
		printf("- 가능한 연산문자 +, -, *, /, 괄호(, )\n");
		printf("- 소수점 자리는 2자리까지 표시됩니다.\n");
		printf("- 허용된 연산문자 외의 문자는 제외하고 계산됩니다. \n");
		printf("- 첫 글자가 종료문자(X, X)인 경우 프로그램이 종료됩니다. \n:");

		int dynamicMemoryIdx = 0, exprSize = 100;
		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			printf("계산식 배열(expression) 메모리 할당 오류 발생.\n\n");
			continue;
		}
		dynamicMemorys[dynamicMemoryIdx] = expression;

		if (!insertExpression(validOperators, exits, &dynamicMemorys[dynamicMemoryIdx], expression, exprSize)) {
			if (strchr(exits, expression[0])) {
				printf("프로그램이 2초 후 종료됩니다.");
				resetProgram(dynamicMemorys, dynamicMemoryCnt);
				Sleep(2000);
				exit(0);
			}
			continue;
		}
		dynamicMemoryIdx++;

		printf("[ 실행 과정 ]\n");
		printf("입력된 계산식(앞/뒤 오타 제외): %s\n", expression);

		if (!validateExpression(expression)) continue;
		printf("- 필터링된 계산식: %s\n", expression);

		if (!normalizeExpression(expression, &dynamicMemorys[dynamicMemoryIdx], (int)strlen(expression))) continue;
		printf("- 보정된 계산식: %s\n", expression);
		dynamicMemoryIdx++;

		double result = 0.0;
		if (!getCalculateResult(expression, &result)) continue;
		printf("계산 결과: %.2f\n\n", result);

		fflush(stdout);
	}

	return 0;
}

// 계산식 입력 (불필요한 앞/뒤 문자 제거 포함)
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

		// 첫 문자 불가 연산자 제거
		if (exprIdx == 0 && strchr("+*/)", ch)) continue;

		if (isdigit(ch) || strchr(validOperators, ch)) {
			// 메모리 재할당
			if (exprIdx + 1 >= exprSize) {
				exprSize *= 2;
				char* temp = (char*)realloc(expression, sizeof(char) * exprSize);
				if (!temp) {
					printf("계산식 배열(expression) 메모리 재할당 오류 발생.\n\n");
					return false;
				}
				expression = temp; // realloc 후 다시 대입
				*dynamicMemory = temp;
			}

			expression[exprIdx] = (char)ch;

			// 마지막 숫자/연산자 위치 갱신
			if (isdigit(ch)) lastDigit = exprIdx;
			if (strchr(validOperators, ch)) lastOp = exprIdx;

			exprIdx++;
		}
	}
	expression[exprIdx] = '\0';

	// 마지막 숫자 이후 불필요한 연산자 제거
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
		printf("입력 중 오류 발생.\n\n");
		return false;
	}

	if (exprIdx == 0) {
		printf("입력된 계산식이 없음.\n\n");
		return false;
	}

	return true;
}

// 계산식 유효성 검사
bool validateExpression(const char* expression) {
	int depth = 0;          // 괄호 깊이
	bool hasDot = false;    // 현재 숫자 내 소수점 존재 여부
	char prev = 0;          // 이전 문자 저장

	for (int i = 0; expression[i] != '\0'; i++) {
		char ch = expression[i];

		// 괄호 짝 검사
		if (ch == '(') depth++;
		else if (ch == ')') depth--;

		if (depth < 0) return false; // 닫힘 괄호 초과

		// 연속 연산자 검사 (예외: +-, --, *-, /-)
		if (prev != '\0' && ch != '\0' && strchr("+-*/", prev) && strchr("+-*/", ch)) {
			if (!(prev == '-' && strchr("*/+", ch))) {
				printf("연산자 연속 오류 \n\n");
				return false;
			}
		}

		// 소수점 검사
		if (ch == '.') {
			if (hasDot || !isdigit(prev)) return false;
			hasDot = true;
		}
		else if (!isdigit(ch)) {
			hasDot = false; // 소수점 포함 상태 리셋
		}

		prev = ch;
	}

	if (depth != 0) {
		printf("괄호 짝 불일치.\n\n");
		return false;
	}

	return true;
}

// 연산식 보정 (괄호, 곱셈 누락/생략 등)
bool normalizeExpression(char* expression, void** dynamicMemory, int exprLen) {
	int exprSize = exprLen + 1;
	char* temp = (char*)malloc(sizeof(char) * (exprSize * 2));
	if (!temp) {
		printf("계산식을 임시 저장할 배열(temp) 메모리 할당 오류.\n\n");
		free(temp);
		return false;
	}

	int j = 0;
	int exprIdx = j;
	// 괄호 및 숫자 간 곱셈 보정
	for (int i = 0; i < exprLen; i++) {
		char ch = expression[i];
		char next = (i + 1 < exprLen) ? expression[i + 1] : '\0';

		// 0) 빈 괄호 제거
		if (ch == '(' && next == ')') {
			i++; // 빈괄호 건너뛰기
			continue;
		}

		// 1) 숫자 바로 뒤 '(' → * 추가
		if (isdigit(ch) && next == '(') {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 2) ')' 바로 뒤 숫자 → * 추가
		else if (ch == ')' && isdigit(next)) {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 3) ')' 바로 뒤 '(' → * 추가
		else if (ch == ')' && next == '(') {
			temp[j++] = ch;
			temp[j++] = '*';
		}

		// 4) '-(' 단항 음수 괄호 처리 → -1*(...)
		else if (ch == '-' && next == '(') {
			temp[j++] = '-';
			temp[j++] = '1';
			temp[j++] = '*';
		}
		else {
			temp[j++] = ch;
		}

		// 메모리 재할당
		if (j + 1 > exprSize) {
			exprSize *= 2;
			char* temptemp = (char*)realloc(temp, sizeof(char) * exprSize);
			if (!temptemp) {
				printf("계산식 배열(expression) 메모리 재할당 오류 발생.\n\n");
				return false;
			}
			temp = temptemp; // realloc 후 다시 대입
			*dynamicMemory = temptemp;
		}
	}
	temp[j] = '\0';
	strcpy(expression, temp);

	j = 0;
	exprLen = (int)strlen(expression);
	// 단항 괄호 제거
	for (int i = 0; i < exprLen; i++) {
		char ch = expression[i];
		if (ch == '(') {
			int innerStart = i + 1;
			int innerEnd = innerStart;

			// 부호 포함 단항
			if (expression[innerStart] == '-' || expression[innerStart] == '+')
				innerEnd++;

			// 숫자 및 소수점
			while (isdigit(expression[innerEnd]) || expression[innerEnd] == '.')
				innerEnd++;

			// 단항 괄호이면 내부 내용만 복사
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

// 계산
bool getCalculateResult(char* expression, double* finalResult)
{
	char* startPtr = NULL;
	char* endPtr = NULL;
	int exprLen = strlen(expression);
	char* innerExpression = (char*)malloc(sizeof(char) * (exprLen + 1));
	if (!innerExpression) {
		printf("괄호 내부 계산식 임시저장 배열(innerExpression) 메모리 재할당 오류 발생.\n\n");
		return false;
	}
	char* tempExpression = (char*)malloc(sizeof(char) * (exprLen * 2 + 1));
	if (!tempExpression) {
		printf("계산식 임시저장 배열(tempExpression) 메모리 재할당 오류 발생.\n\n");
		return false;
	}

	strcpy(tempExpression, expression);
	while ((startPtr = strrchr(expression, '(')) != NULL)
	{
		// 2 - 1. 닫힘 괄호가 나오기 전까지를 배열에 담아 계산 함수에 보낸다. 
		endPtr = strchr(startPtr, ')');
		if (!endPtr) {
			printf("닫힘 괄호가 없음.\n\n");
			return false;
		}

		// 내부 계산식 추출
		int innerLen = endPtr - startPtr - 1;
		strncpy(innerExpression, startPtr + 1, innerLen);
		innerExpression[innerLen] = '\0';

		double result = calculate(innerExpression, strlen(innerExpression));

		// 3. 임시 계산식 배열에 열림 괄호 전, 계산 결과, 닫힘괄호 후 문자들을 합쳐 저장한다. 
		sprintf(tempExpression, "%.*s%.2f%s",
			(int)(startPtr - expression),
			expression,
			result,
			endPtr + 1
		);
		strcpy(expression, tempExpression);
	}

	// 2 - 2. 만약 괄호가 없다면 전체를 배열에 담아 계산 함수에 보낸다.
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

		// 음수 부호 처리 (처음이거나 이전이 연산자일 때)
		if (ch == '-' && (i == 0 || strchr("+-*/", expression[i - 1]))) {
			isNegative = true;
			i++;
			continue;
		}

		// 숫자 처리
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

		// 연산자 처리
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

	// 남은 연산 처리
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
