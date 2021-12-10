#ifndef SME_H
#define SME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_SIZE 256

/* SME NODE */
enum SMEType {
    SMENum,
    SMEAdd,
    SMESub,
    SMEMul,
    SMEDiv,
    SMENeg,
    SMEPos,
    SMELP,
    SMERP,
    SMEFloor,
    SMECeil
};

typedef struct SMENode {
    enum SMEType type;
    double value;
    struct SMENode* left;
    struct SMENode* right;
} SMENode;


/* SME TOKEN */
typedef struct SMEToken {
    enum SMEType type;
    double value;
} SMEToken;


/* SME LIST */
typedef struct SMEList {
    int heap_size;
    int count;
    void** items;
} SMEList;


/* SME Variable */
typedef struct SMEVar {
    char* name;
    double value;
} SMEVar;


/* SME TOKENIZER */
typedef struct SMETokenizer {
    char* buffer;
    char* temp;
    int idx;
    int tidx;
    SMEList* list;
    SMEList* variables;
    SMEToken* current;
} SMETokenizer;


/* NODE IMPLEMENTATION */
SMENode* new_SMENode(enum SMEType type) {
    SMENode* node = (SMENode*)malloc(sizeof(SMENode));
    node->type = type;
    node->value = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void free_SMENode(SMENode* node) {
    if(node->left)
        free_SMENode(node->left);
    if (node->right)
        free_SMENode(node->right);
    free(node);
}

void print_SMENode(SMENode* node) {
    if (node) {
        if(node->left)
            print_SMENode(node->left);

        if (node->type == SMEAdd) {
            printf("+");
        } else if (node->type == SMESub) {
            printf("-");
        } else if (node->type == SMEMul) {
            printf("*");
        } else if (node->type == SMEDiv) {
            printf("/");
        } else if (node->type == SMEPos) {
            printf("pos");
        } else if (node->type == SMENeg) {
            printf("neg");
        } else if (node->type == SMENum) {
            printf("%.2lf", node->value);
        }

        if (node->right)
            print_SMENode(node->right);
    }
}

/* TOKEN IMPLEMENTATION */
SMEToken* new_SMEToken(enum SMEType type) {
    SMEToken* token = (SMEToken*)malloc(sizeof(SMEToken));
    token->type = type;
    token->value = 0;
    return token;
}


/* LIST IMPLEMENTATION */
SMEList* new_SMEList() {
    SMEList* list = malloc(sizeof(SMEList));
    if (!list) {
        printf("cannot allocate list\n");
        exit(1);
    }
    list->count = 0;
    list->heap_size = LIST_SIZE;
    list->items = malloc(sizeof(void*) * list->heap_size);
    return list;
}

void append_SMEItem(SMEList* list, void* item) {
    if (list->count >= list->heap_size) {
        list->heap_size *= 2;
        list->items = realloc(list->items, sizeof(void*) * list->heap_size);
        if (list->items == NULL) {
            printf("Unable to reallocate list\n");
        }
    }
    list->items[list->count++] = item;
}

void free_SMEList(SMEList* list) {
    if (list) {
        free(list->items);
        free(list);
    }
}


/* VARIABLE IMPLEMENTATION */
SMEVar* new_SMEVar(const char* name, double value) {
    SMEVar* var = malloc(sizeof(SMEVar));
    var->name = malloc(sizeof(char) * strlen(name) + 1);
    strcpy(var->name, name);
    var->value = value;
    return var;
}

void free_SMEVar(SMEVar* var) {
    free(var->name);
    free(var);
}


/* TOKENIZER IMPLEMENTATION */
SMETokenizer* new_SMETokenizer(char* buffer) {
    SMETokenizer* tokenizer = (SMETokenizer*) malloc(sizeof(SMETokenizer));
    tokenizer->buffer = buffer;
    tokenizer->temp = (char*) malloc(sizeof(char) * 256);
    tokenizer->idx = 0;
    tokenizer->tidx = 0;
    tokenizer->list = new_SMEList();
    tokenizer->variables = NULL;
    tokenizer->current = NULL;

    return tokenizer;
}

void free_SMETokenizer(SMETokenizer* tokenizer) {
    free(tokenizer->temp);
    if(tokenizer->list != NULL){
        for (int i = 0; i < tokenizer->list->count; i++) {
            free(tokenizer->list->items[i]);
        }
        free_SMEList(tokenizer->list);
    }
    if(tokenizer->variables != NULL){
        for (int i = 0; i < tokenizer->variables->count; i++) {
            free_SMEVar(tokenizer->variables->items[i]);
        }
        free_SMEList(tokenizer->variables);
    }
    free(tokenizer);
}

void advance_SMETokenizer(SMETokenizer* tokenizer) {
    if (tokenizer->tidx + 1 <= tokenizer->list->count)
        tokenizer->current = tokenizer->list->items[tokenizer->tidx++];
    else
        tokenizer->current = NULL;
}


/* HELPER FUNCTIONS */
int is_digit(const char c) {
    if (c >= 0x30 && c <= 0x39) return 1;
    return 0;
}

int is_alpha(char c) {
    if ((c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A)) return 1;
    return 0;
}


/* TOKENIZER */
void sme_tokenize_number(SMETokenizer* tokenizer) {
    SMEToken* token = NULL;
    if (is_digit(tokenizer->buffer[tokenizer->idx])) {
        /* Load each digit in the number into the temp buffer */
        while (is_digit(tokenizer->buffer[tokenizer->idx])) {
            tokenizer->temp[tokenizer->tidx++] = tokenizer->buffer[tokenizer->idx++];
            /* In case we have a floating point value keep the dot */
            if (tokenizer->buffer[tokenizer->idx] == '.')
                tokenizer->temp[tokenizer->tidx++] = tokenizer->buffer[tokenizer->idx++];
        }
        tokenizer->temp[tokenizer->tidx] = '\0';

        token = new_SMEToken(SMENum);
        token->value = strtod(tokenizer->temp, NULL);

        append_SMEItem(tokenizer->list, token);

        tokenizer->tidx = 0;
    }
}

void sme_tokenize_string(SMETokenizer* tokenizer) {
    SMEToken* token = NULL;
    if (is_alpha(tokenizer->buffer[tokenizer->idx])) {
        tokenizer->tidx = 0;
        /* Load the variable / function name into the temp buffer */
        while (is_alpha(tokenizer->buffer[tokenizer->idx])) {
            tokenizer->temp[tokenizer->tidx++] = tokenizer->buffer[tokenizer->idx++];
        }
        tokenizer->temp[tokenizer->tidx] = '\0';

        if (!strcmp(tokenizer->temp, "floor\0")) {
            token = new_SMEToken(SMEFloor);
            append_SMEItem(tokenizer->list, token);
        }
        else if (!strcmp(tokenizer->temp, "ceil\0")) {
            token = new_SMEToken(SMECeil);
            append_SMEItem(tokenizer->list, token);
        }
        else {
            for (int i = 0; i < tokenizer->variables->count; i++) {
                SMEVar* var = tokenizer->variables->items[i];
                if (!strcmp(tokenizer->temp, var->name)) {
                    token = new_SMEToken(SMENum);
                    token->value = var->value;
                    append_SMEItem(tokenizer->list, token);
                }
            }
        }
        tokenizer->tidx = 0;
    }
}

void sme_tokenize_operator(SMETokenizer* tokenizer) {
    SMEToken* token = NULL;
    if (tokenizer->buffer[tokenizer->idx] == '+') {
        token = new_SMEToken(SMEAdd);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
    else if (tokenizer->buffer[tokenizer->idx] == '-') {
        token = new_SMEToken(SMESub);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
    else if (tokenizer->buffer[tokenizer->idx] == '/') {
        token = new_SMEToken(SMEDiv);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
    else if (tokenizer->buffer[tokenizer->idx] == '*') {
        token = new_SMEToken(SMEMul);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
    else if (tokenizer->buffer[tokenizer->idx] == '(') {
        token = new_SMEToken(SMELP);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
    else if (tokenizer->buffer[tokenizer->idx] == ')') {
        token = new_SMEToken(SMERP);
        token->value = strtod(tokenizer->temp, NULL);
        append_SMEItem(tokenizer->list, token);
    }
}

SMETokenizer* sme_tokenize(char* buffer, SMEList* variables) {
    SMETokenizer* tokenizer = new_SMETokenizer(buffer);
    tokenizer->variables = variables;
    while (tokenizer->buffer[tokenizer->idx]) {
        sme_tokenize_number(tokenizer);
        sme_tokenize_string(tokenizer);
        sme_tokenize_operator(tokenizer);
        tokenizer->idx++;
    }
    tokenizer->tidx = 0;
    return tokenizer;
}


/* PARSER*/
SMENode* sme_term(SMETokenizer* tokenizer);
SMENode* sme_expr(SMETokenizer* tokenizer);

SMENode* sme_factor(SMETokenizer* tokenizer) {
    SMEToken* token = tokenizer->current;
    SMENode* result;
    if(token != NULL) {
        if (token->type == SMELP) {
            advance_SMETokenizer(tokenizer);
            result = sme_expr(tokenizer);
            if (tokenizer->current->type != SMERP) printf("Missing right parenthesis\n");
            advance_SMETokenizer(tokenizer);
            return result;
        } else if (token->type == SMENum){
            advance_SMETokenizer(tokenizer);
            result = new_SMENode(SMENum);
            result->value = token->value;
            return result;
        } else if (token->type == SMESub) {
            advance_SMETokenizer(tokenizer);
            result = new_SMENode(SMENeg);
            result->left = sme_factor(tokenizer);
            return result;
        } else if (token->type == SMEAdd) {
            advance_SMETokenizer(tokenizer);
            result = new_SMENode(SMEPos);
            result->left = sme_factor(tokenizer);
            return result;
        } else if (token->type == SMEFloor) {
            advance_SMETokenizer(tokenizer);
            result = new_SMENode(SMEFloor);
            result->left = sme_factor(tokenizer);
            return result;
        } else if (token->type == SMECeil) {
            advance_SMETokenizer(tokenizer);
            result = new_SMENode(SMECeil);
            result->left = sme_factor(tokenizer);
            return result;
        }
    }
    return NULL;
}

SMENode* sme_term(SMETokenizer* tokenizer) {
    SMENode* result = sme_factor(tokenizer);
    SMENode* temp = NULL;
    while (tokenizer->current != NULL && (tokenizer->current->type == SMEMul || tokenizer->current->type == SMEDiv)) {
        temp = result;
        result = new_SMENode(tokenizer->current->type);
        result->left = temp;
        advance_SMETokenizer(tokenizer);
        result->right = sme_term(tokenizer);
    }
    return result;
}

SMENode* sme_expr(SMETokenizer* tokenizer) {
    SMENode* result = sme_term(tokenizer);
    SMENode* temp = NULL;

    while (tokenizer->current != NULL && (tokenizer->current->type == SMEAdd || tokenizer->current->type == SMESub)) {
        temp = result;
        result = new_SMENode(tokenizer->current->type);
        result->left = temp;
        advance_SMETokenizer(tokenizer);
        result->right = sme_term(tokenizer);
    }
    return result;
}

SMENode* sme_parse(SMETokenizer* tokenizer) {
    advance_SMETokenizer(tokenizer);
    return sme_expr(tokenizer);
}


/* MATH */
double floor(double value) {
    if (value < 0) {
        return (int)(value + (-1));
    }
    return (int)value;
}

double ceil(double value) {
    if (value - (int)value == 0) return value;
    return floor(value) + 1;
}


/* EVALUATION */
double sme_eval(SMENode* node) {
    double res = 0;
    double left;
    double right;
    if (node->type == SMENum) {
        res = node->value;
        return res;
    } else if (node->type == SMEAdd) {
        left = sme_eval(node->left);
        right = sme_eval(node->right);
        res = left + right;
        return res;
    } else if (node->type == SMESub) {
        left = sme_eval(node->left);
        right = sme_eval(node->right);
        res = left - right;
        return res;
    } else if (node->type == SMEMul) {
        left = sme_eval(node->left);
        right = sme_eval(node->right);
        res = left * right;
        return res;
    } else if (node->type == SMEDiv) {
        left = sme_eval(node->left);
        right = sme_eval(node->right);
        res = left / right;
        return res;
    }
    else if (node->type == SMENeg) {
        left = sme_eval(node->left);
        res = -left;
        return res;
    }
    else if (node->type == SMEPos) {
        left = sme_eval(node->left);
        res = left > 0 ? left : -left;
        return res;
    }
    else if (node->type == SMEFloor) {
        left = sme_eval(node->left);
        res = floor(left);
        return res;
    }
    else if (node->type == SMECeil) {
        left = sme_eval(node->left);
        res = ceil(left);
        return res;
    }
    return res;
}

double sme_calc(char* buffer, SMEList* variables) {
    SMETokenizer* tokenizer = sme_tokenize(buffer, variables);
    SMENode* root = sme_parse(tokenizer);
    double res = sme_eval(root);
    free_SMENode(root);
    free_SMETokenizer(tokenizer);
    return res;
}
#endif //SME_H
