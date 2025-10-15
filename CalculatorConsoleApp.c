#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#define _CRT_SECURE_NO_WARNINGS
//123 + 2 * 2 + (3 + 2) -> 129 || (3+2) -> 3
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
void* dynamicArrays[3] = { NULL, 
NULL, NULL };	// 동적 배열 메모리 한번에 관리
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
		printf("[ 설명 ]\n");
		printf("계산할 연산식을 입력하시오(종료문자 x 혹은 mX) ex) 1+2*3\n");
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
		printf("\n[계산 결과]\n");
		printf("- 필터링된 계산식: %s\n", expression);
		printf("-- 연산자:");
        for (int i = 0; i < data.operatorLen; i++) printf("%c ", operators[i]);
        printf("\n-- 피연산자:");
        for (int i = 0; i < data.numberLen; i++) printf("%d ", numbers[i]);

		int result = 0;
		status = calculate(&data, &result);
		if (status == STATUS_RESTART) {
			continue;
		}

		printf("\n==> 계산 결과: %d\n\n", result);
		executeProgramControl(STATUS_RESTART, "");
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

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

int applyOp(int a, int b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/': 
        if (b == 0) {
            printf("0으로 나눌 수 없습니다.\n");
            return 0;
        }
        return a / b;
    }
    return 0;
}

ProcessRequestStatus calculate(ExpressionData* data, int* result)
{
	const char* s = data->expression;
	int values[256]; int vTop = -1;
	char ops[256]; int oTop = -1;

	for (int i = 0; s[i]; ) {
		if (isspace((unsigned char)s[i])) { i++; continue; }

		if (isdigit((unsigned char)s[i])) {
			int val = 0;
			while (isdigit((unsigned char)s[i])) {
				val = val * 10 + (s[i] - '0');
				i++;
			}
			values[++vTop] = val;
		}
		else if (s[i] == '(') {
			ops[++oTop] = s[i];
			i++;
		}
		else if (s[i] == ')') {
			while (oTop >= 0 && ops[oTop] != '(') {
				int b = values[vTop--];
				int a = values[vTop--];
				char op = ops[oTop--];
				values[++vTop] = applyOp(a, b, op);
			}
			oTop--; // '(' 제거
			i++;
		}
		else if (strchr("+-*/", s[i])) {
			while (oTop >= 0 && precedence(ops[oTop]) >= precedence(s[i])) {
				int b = values[vTop--];
				int a = values[vTop--];
				char op = ops[oTop--];
				values[++vTop] = applyOp(a, b, op);
			}
			ops[++oTop] = s[i];
			i++;
		}
		else {
			i++; // 무시
		}
	}

	while (oTop >= 0) {
		int b = values[vTop--];
		int a = values[vTop--];
		char op = ops[oTop--];
		values[++vTop] = applyOp(a, b, op);
	}

	*result = values[vTop];
	return STATUS_PROCESS;
}