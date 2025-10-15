#include <stdio.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS


int main()
{

	printf("== 계산기 프로그램 ==\n");

	while (1)
	{
	HERE:
		printf("계산할 연산식을 입력하시오(종료문자 x 혹은 mX) ex) 1+2*3\n");
		printf("- 가능한 연산문자 +, -, *, /\n");
		printf("- 허용된 연산문자 외의 문자는 제외하고 계산됩니다. \n");
		printf("- 첫 글자가 종료문자(X, X)인 경우 프로그램이 종료됩니다. \n:");

		int input = 0;
		boolean isFirst = TRUE;
		char* validOperators = "*/+-";
		char* exits = "xX";
		size_t exprLen = 0;
		size_t exprSize = 100;

		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			printf("메모리 할당 실패. 관리자 문의 필요\n");
			return 0;
		}

		// [4-2] === 사용자 입력 및 저장 ===
		while ((input = getchar()) != '\n' && input != EOF) {
			// 첫 문자 종료키 처리
			if (isFirst) {
				if (strchr(exits, input)) {
					printf("프로그램 종료\n");
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
						printf("메모리 재할당 실패. 관리자 문의 필요\n");
						free(expression);
						goto HERE;
					}
					expression = temp;
				}
				expression[exprLen++] = (char)input;
			}
		}

		if (input == EOF && ferror(stdin)) {
			printf("입력 값 읽어오는 중 오류 발생. 재입력 해주세요.");
			clearerr(stdin);
			free(expression);
			continue;
		}

		expression[exprLen] = '\0';
		if (exprLen == 0) {
			printf("저장된 계산식이 없습니다. 다시 입력해주세요.\n\n");
			continue;
		}

		// [5번] === 저장된 계산식 정리 ===
		exprLen = strlen(expression);
		// 앞 연산자 제거
		while (exprLen > 0 && strchr(validOperators, expression[0])) {
			memmove(expression, expression + 1, strlen(expression) + 1);
			exprLen--;
		}

		// 뒤 연산자 제거
		while (exprLen > 0 && strchr(validOperators, expression[exprLen - 1])) {
			expression[--exprLen] = '\0';
		}

		// 저장된 값이 없음 오류
		if (exprLen == 0) {
			printf("계산식 정리시 계산식이 올바르지 않습니다. 다시 입력해주세요.\n\n");
			continue; //입력으로 돌아감
		}

		// 연산자 연속 오류
		for (int exprIdx = 1; exprIdx < exprLen; ++exprIdx) {
			if (strchr(validOperators, expression[exprIdx - 1]) && strchr(validOperators, expression[exprIdx])) {
				printf("계산식 정리 중 오타로 인한 연산자 연속. 재입력 요청\n\n");
				goto HERE;
			}
		}

		// 숫자 및 연산자 분리		
		int* numbers = (int*)malloc(sizeof(int) * exprLen);
		char* operators = (char*)malloc(sizeof(char) * exprLen);

		if (!numbers || !operators) {
			printf("메모리 할당 실패\n");
			free(numbers);
			free(operators);
			goto HERE; // 입력으로 복귀
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
			printf("계산에 필요한 숫자 혹은 연산자가 부족.\n\n");
			continue;
		}

		// 계산 처리
		int i = 0;
		while (i < operatorIdx) {
			if (operators[i] == '*' || operators[i] == '/') {
				int a = numbers[i];
				int b = numbers[i + 1];

				if (operators[i] == '/' && b == 0) {
					printf("0으로 나눌 수 없습니다\n\n");
					goto HERE; //입력으로 돌아감
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

		printf("필터링된 계산 수식: %s\n", expression);
		int result = numbers[0];
		for (int j = 0; j < operatorIdx; j++) {
			if (operators[j] == '+') result += numbers[j + 1];
			else result -= numbers[j + 1];
		}

		printf("계산 결과: %d \n\n", result);

	}

	return 1;
}