#include <stdio.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS

// 프로그램 처리 요청 상태
typedef enum {
	STATUS_PROCESS,		 // 다음 로직 실행 (정상 실행)
	STATUS_FORCE_EXIT,	 // 강제 종료 (오류)
	STATUS_EXIT,		 // 정상 종료
	STATUS_RESTART       // 프로그램 재시작
} ProcessRequestStatus;

const char* validOperators = "*/+-"; // TODO ( 와 ) 추가 하기
const char* exits = "xX";

// executeProgramControl
// 함수 정의: 프로그램 처리 요청 상태(ProcessRequestStatus)를 받아 각 상태에 따라 프로그램 종료 혹은 재 실행.
//	- 프로그램 재실행은 함수 내에서 처리가 안되므로 재시작 값 반환
//	- 프로그램 처리 요청 상태(종료/재시작)와, 출력문을 받고 처리한다.
//		- 공통: 동적 메모리 해제를 통한 메모리 누수 차단	
//		- 프로그램 재시작: 
//			1) 재시작 문구 출력
//			2) 프로그램 재시작 상태 값 ProcessRequestStatus::STATUS_RESTART 반환
//		- 프로그램 비정상 종료
//			1) 종료 문구 출력
//			2) 종료 딜레이 (sleep(2000))
//			3) 종료 exit(1);
//		- 프로그램 정상 종료
//			1) 종료 문구 출력
//			2) 종료 딜레이 (sleep(2000))
//			3) 종료 exit(0);
ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context);

// validateExpression
// 함수 정의: 사용자 입력 값을 필터링하며, 오류가 발생하는 경우 처리 요청 상태를 반환한다.
// - 잘못된 입력값 필터링
//	1) 앞 연산자 제거
//		- 연산자가 열림 괄호일 경우 미제거
//	2) 뒤 연산자 제거
//		- 연산자가 닫힘 괄호일 경우 미제거
//	3) 저장된 값이 있는지 체크
//		- 저장된 값이 없을 경우 재시작 요청 상태를 반환한다.
//	4) 연산자 연속 입력 오류 체크
//		- 잘못된 입력이였을 경우 재시작 요청 상태를 반환한다.
//  5) TODO 괄호가 쌍을 이루는지 오류 체크
//		- 열림 닫침 (, ) 괄호가 쌍으로 있어야한다.
//		- 잘못된 입력이였을 경우 재시작 요청 상태를 반환한다.
// - 정상적으로 처리되었을 경우 다음 로직 실행 상태를 반환한다.
ProcessRequestStatus validateExpression(int* exprLen, char* expression);

// parseExpression
// 함수 정의: 연산식을 받아서 연산자와 피연산자를 연산 순서대로 각각 나눠담는 함수
//	- TODO 괄호도
ProcessRequestStatus parseExpression(int numbers[], char operators[], int* numberIdx, int* operatorIdx);

// 함수 정의: 연산자와 피연산자 배열을 순회하여 계산하고 결과값을 구하는 함수
//	- 결과값을 리턴한다.
//	1) 괄호를 안에 있는 값을 가장 먼저 계산 진행
//		- 괄호를 만나면 재귀 호출로 안쪽부터 계산
//	2) 곱셈 나눗셈 계산 먼저 진행
//		- 나눗셈 오류 체크 (나눗셈 연산 중 0으로 나누는 경우 오류발생)
//		- 잘못된 입력이였을 경우 재시작 요청 상태를 반환한다.
//	3) 덧셈 뺄셈 계산 진헹 
int calculate(char* expression);

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
		int ch = 0;
		while ((ch = getchar()) != '\n' && ch != EOF) {
			// 첫 문자 종료키 처리
			if (isFirst) {
				if (strchr(exits, ch)) {
					printf("프로그램 종료\n");
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
						printf("메모리 재할당 실패. 관리자 문의 필요\n");
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
			printf("입력 값 읽어오는 중 오류 발생. 재입력 해주세요.");
			clearerr(stdin);
			free(expression);
			continue;
		}

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
		while (*expressionPtr) { // 표현식이 존재할때 까지
			char* endptr;
			int num = strtol(expressionPtr, &endptr, 10);
			
			// 포인터가 가리키는 값이 숫자인 경우
			if (isdigit((unsigned char)*expressionPtr)) {
				numbers[numberIdx] = num;
				expressionPtr = endptr;
		
				// 숫자 다음 포인터 값이 연산자인 경우
				if (*expressionPtr && strchr(validOperators, *expressionPtr)) {
					operators[operatorIdx] = *expressionPtr;
					expressionPtr++;
					operatorIdx++;
				}
				numberIdx++;
			} 
			//else if ( TODO 괄호 추가 )
			else {
				break;
			}
		}

		int numberCount = numberIdx;
		int operatorCount = operatorIdx;

		if (numberCount < 2 || operatorCount < 1) {
			printf("계산에 필요한 숫자 혹은 연산자가 부족. 재입력 해주세요.\n\n");
			continue;
		}

		// [7번] === 계산식 출력 및 계산 ===
		printf("필터링된 계산식: %s\n", expression);

		// 계산 처리
		int i = 0;
		while (i < operatorCount) {
			if (operators[i] == '*' || operators[i] == '/') {
				int a = numbers[i];
				int b = numbers[i + 1];

				if (operators[i] == '/' && b == 0) {
					printf("0으로 나눌 수 없습니다\n\n");
					goto HERE; //입력으로 돌아감
				}

				// i 와 i+1 의 계산 결과값을 i에 저장
				numbers[i] = (operators[i] == '*') ? a * b : a / b;

				// i+1 부터 좌 shift 
				for (int j = i + 1; j < numberCount - 1; j++)
				{
					numbers[j] = numbers[j + 1];
				}

				// i 부터 좌 shift
				for (int j = i; j < operatorCount - 1; j++)
				{
					operators[j] = operators[j + 1];
				}

				// 줄인 범위에서 반복되도록 변경
				numberCount--;
				operatorCount--;
			}
			else {
				// 곱셈, 나눗셈이 아닌 경우 다음 연산자 조회
				i++;
			}
		}

		// 덧셈, 뺄셈 계산
		int result = numbers[0];
		for (int j = 0; j < operatorCount; j++) {
			if (operators[i] == '+') {
				result += numbers[i + 1];
			}
			else {
				result -= numbers[i + 1];
			}
		}

		printf("계산 결과: %d \n\n", result);

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