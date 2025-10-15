#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS

// enum, struct, static 변수
typedef enum {
	STATUS_PROCESS,		 // 다음 로직 실행 (정상 실행)
	STATUS_FORCE_EXIT,	 // 강제 종료 (오류)
	STATUS_EXIT,		 // 정상 종료
	STATUS_RESTART       // 프로그램 재시작
} ProcessRequestStatus;
typedef struct {
	const char* expression; // 원본 수식
	int* numbers;           // 피연산자 배열
	int  numberLen;         // 피연산자 개수
	char* operators;        // 연산자 배열
	int  operatorLen;       // 연산자 개수
} ExpressionData;
void* dynamicArrays[3] = { NULL, NULL, NULL };	// 동적 배열 메모리 한번에 관리
const char* validOperators = "*/+-()";
const char* validBasicOperators = "*/+-";
const char* exits = "xX";

// 함수 선언
ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context);
ProcessRequestStatus validateExpression(int *exprLen, char* expression);
ProcessRequestStatus parseExpression(ExpressionData* data);
ProcessRequestStatus calculate(ExpressionData* data, int* result);

int main()
{
	ProcessRequestStatus status;

	printf("== 계산기 프로그램 ==\n");

	while (1)
	{
		printf("계산할 연산식을 입력하시오(종료문자 x 혹은 mX) ex) 1+2*3\n");
		printf("[ 설명 ]\n");
		printf("- 가능한 연산문자 +, -, *, /\n");
		printf("- 허용된 연산문자 외의 문자는 제외하고 계산됩니다. \n");
		printf("- 첫 글자가 종료문자(X, X)인 경우 프로그램이 종료됩니다. \n:");
		
		int exprLen = 0, exprSize = 100;
		char* expression = (char*)malloc(sizeof(char) * exprSize);
		if (!expression) {
			executeProgramControl(STATUS_FORCE_EXIT,"메모리 할당 실패. 관리자 문의 필요\n");
		}

		dynamicArrays[0] = expression;

		// == 사용자 입력 ==
		bool isFirst = TRUE;
		int ch;
		while ((ch = getchar()) != '\n' && ch != EOF) {
			if (isFirst) {
				if (strchr(exits, ch)) {
					executeProgramControl(STATUS_EXIT, "사용자 종료 요청");
				}
				isFirst = FALSE;
			}

			if (isdigit(ch) || strchr(validOperators, ch)) {
				if (exprLen + 1 >= exprSize) {
					exprSize *= 2;
					char* temp = (char*)realloc(expression, sizeof(char) * exprSize);
					if (!temp) {
						executeProgramControl(STATUS_FORCE_EXIT, "메모리 재할당 실패. 관리자 문의 필요\n");
					}
					expression = temp;
					dynamicArrays[0] = expression; // realloc 후 다시 대입
				}
				expression[exprLen++] = (char)ch;
			}
		}
		expression[exprLen] = '\0';

		if (ch == EOF && ferror(stdin)) {
			clearerr(stdin);
			executeProgramControl(STATUS_RESTART, "입력 값 읽어오는 중 오류 발생.재입력 해주세요.\n");
			continue;
		}
		if (exprLen == 0) {
			executeProgramControl(STATUS_RESTART, "저장된 계산식이 없습니다. 다시 입력해주세요.\n\n");
			continue;
		}

		// == 수식 필터링 ==
		exprLen = strlen(expression);
		status = validateExpression(&exprLen, expression);
		if (status == STATUS_RESTART) {
			continue;
		}

		// == 연산자, 피연산자 배열 분리 ==
		int* numbers = (int*)malloc(sizeof(int) * exprLen);
		char* operators = (char*)malloc(sizeof(char) * exprLen);
		if (!numbers || !operators) {
			executeProgramControl(STATUS_RESTART, "메모리 할당 실패\n\n");
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

		// === 계산식 출력 및 계산 ===
		printf("필터링된 계산식: %s\n", expression);
		int result = 0;
		status = calculate(&data, &result);
		if (status == STATUS_RESTART) {
			continue;
		}

		printf("계산 결과: %d\n\n", result);

	}

	return 1;
}

ProcessRequestStatus executeProgramControl(ProcessRequestStatus status, char* context)
{
	if (context)
		printf("%s\n", context);

	// 동적 메모리 해제 (TODO 이후 가변적으로 처리)
	for (int i = 0; i < 3; i++) {
		if (dynamicArrays[i]) {
			free(dynamicArrays[i]);
			dynamicArrays[i] = NULL;
		}
	}

	switch (status)
	{
		case STATUS_FORCE_EXIT:
			// 강제 종료 (오류)
			printf("프로그램이 2초 후 강제 종료됩니다.");
			Sleep(2000);
			exit(1);
			break;
		
		case STATUS_EXIT:
			// 정상 종료
			printf("프로그램이 2초 후 정상 종료됩니다.");
			Sleep(2000);
			exit(0);
			break;

		case STATUS_RESTART:
			// 프로그램 재시작
			printf("=== 프로그램 재시작 ===\n");
			return STATUS_RESTART;

		default: 
			return STATUS_PROCESS;
	}
}

ProcessRequestStatus validateExpression(int* exprLen, char* expression)
{
	// 앞 연산자 제거
	while (*exprLen > 0 && strchr(validOperators, expression[0])) {
		if (expression[0] == '(') break;
		memmove(expression, expression + 1, strlen(expression));
		(*exprLen)--;
	}

	// 뒤 연산자 제거
	while (*exprLen > 0 && strchr(validOperators, expression[*exprLen - 1])) {
		if (expression[*exprLen - 1] == ')') break;
		expression[--(*exprLen)] = '\0';
	}

	// 저장된 값이 없음 오류
	if (*exprLen == 0) {
		return executeProgramControl(STATUS_RESTART, "계산식 정리 중 계산식이 비어 있습니다. 다시 입력해주세요.\n\n");
	}

	// 연산자 연속 오류 검사
	for (int i = 1; i < *exprLen; ++i) {
		if (strchr(validBasicOperators, expression[i - 1]) && strchr(validBasicOperators, expression[i])) {
			return executeProgramControl(STATUS_RESTART, "계산식 정리 중 오타로 인한 연산자 연속. 재입력 요청\n\n");
		}
	}

	// 괄호 검사
	// 1. ( 뒤: 숫자, 또 다른 '(' , 단항 +/-
	// 2. ) 뒤: 또 다른 ) 또는 '(' 또는 기본 연산자
	// 3. 숫자 바로 뒤에 여는 괄호: 곱셈 생략 (예: 3(2+1)) _ 오류발생으로 보류 

	int balance = 0;
	for (int i = 0; i < *exprLen; ++i) {
		if (expression[i] == '(')
			balance++;
		else if (expression[i] == ')')
			balance--;

			if (balance < 0) {
			return executeProgramControl(STATUS_RESTART, "닫는 괄호 ')'가 너무 많습니다. 수식을 다시 입력해주세요.\n\n");
		}
	}

	if (balance != 0) {
		return executeProgramControl(STATUS_RESTART, "괄호 짝이 맞지 않습니다. 다시 입력해주세요.\n\n");
	}

	return STATUS_PROCESS;

}

ProcessRequestStatus parseExpression(ExpressionData* data)
{
	char* expressionPtr = data->expression;
	while (*expressionPtr) { // 표현식이 존재할때 까지
		char* endptr;
		int num = strtol(expressionPtr, &endptr, 10);

		// 포인터가 가리키는 값이 숫자인 경우
		if (isdigit((unsigned char)*expressionPtr)) {
			data->numbers[data->numberLen++] = num;
			expressionPtr = endptr;

			// 숫자 뒤에 연산자가 있으면 추가
			if (*expressionPtr && strchr(validOperators, *expressionPtr)) {
				data->operators[data->operatorLen++] = *expressionPtr;
				expressionPtr++;
			}
		} 
		else if (strchr(validOperators, *expressionPtr)) {
			// 연산자 또는 괄호
			data->operators[data->operatorLen++] = *expressionPtr;
			expressionPtr++;
		/*else if (*expressionPtr == '(' || *expressionPtr == ')') { 
			data->operators[data->operatorLen++] = *expressionPtr;
			expressionPtr++;*/
		} else {
			return executeProgramControl(STATUS_RESTART, "공백 또는 잘못된 문자 발견.\n\n");
		}
	}

	if (data->numberLen < 2 || data->operatorLen < 1) {
		return executeProgramControl(STATUS_RESTART, "계산에 필요한 숫자 혹은 연산자가 부족. 재입력 해주세요\n\n");
	}
	
	data->operators[data->operatorLen] = '\0';
	return STATUS_PROCESS;
}

ProcessRequestStatus calculate(ExpressionData* data, int* result)
{
	int i = 0;
	// 괄호 계산
	while (i < data->operatorLen) {
		if (data->operators[i] == '(') {
			int depth = 1;
			int j = i + 1;

			// 짝이 맞는 ')' 찾기
			while (depth > 0) {
				if (data->operators[j] == '(') {
					depth++;
				}
				else if (data->operators[j] == ')') {
					depth--;
				}
				j++;
			}

			// i~j-1 구간이 괄호 내부
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

			// 결과를 numbers[i]에 덮어쓰기
			data->numbers[i] = innerResult;

			// 괄호 구간 삭제 후 시프트
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
			continue; // 현재 위치 다시 검사 (중첩 괄호 가능)
		}
		i++;

	}

	//곱셈 나눗셈
	i = 0;
	while (i < data->operatorLen) {
		if (data->operators[i] == '*' || data->operators[i] == '/') {
			int a = data->numbers[i];
			int b = data->numbers[i + 1];

			if (data->operators[i] == '/' && b == 0) {
				executeProgramControl(STATUS_RESTART, "0으로 나눌 수 없습니다\n\n");
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
			i++; // 곱셈, 나눗셈이 아닌 경우 다음 연산자 조회
		}
	}

	// 덧셈, 뺄셈 계산
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