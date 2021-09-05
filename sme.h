enum SMEType {
	SMENum,
	SMEAdd,
	SMESub,
	SMEMul,
	SMEDiv,
	SMENeg,
	SMELP,
	SMERP,
	SMENull,
};

typedef struct _SMENode {
	enum SMEType type;
	double value;
	struct _SMENode* left;
	struct _SMENode* right;
} SMENode;

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

typedef struct _SMEToken {
	enum SMEType type;
	double value;
} SMEToken;

SMEToken* new_SMEToken(enum SMEType type) {
	SMEToken* token = (SMEToken*)malloc(sizeof(SMEToken));
	token->type = type;
	token->value = 0;
	return token;
}

/* XML LIST */
typedef struct _SMEList {
	int heap_size;
	int count;
	void** items;
} SMEList;

/* LIST IMPLEMENTATION */
SMEList* new_SMEList() {
	SMEList* list = malloc(sizeof(SMEList));
	if (!list) {
		printf("cannot allocate list\n");
		exit(1);
	}
	list->count = 0;
	list->heap_size = NODE_SIZE;
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


typedef struct _SMETokens {
	char* buffer;
	char* temp;
	int idx;
	int tidx;
	SMEList* list;
	SMEToken* current;
} SMETokens;

SMETokens* new_SMETokens(char* buffer) {
	SMETokens* tokens = (SMETokens*) malloc(sizeof(SMETokens));
	tokens->buffer = buffer;
	tokens->temp = (char*) malloc(sizeof(char) * 256);
	tokens->idx = 0;
	tokens->tidx = 0;
	tokens->list = new_SMEList();
	tokens->current = NULL;

	return tokens;
}

void free_SMETokens(SMETokens* tokens) {
	free(tokens->temp);
	for (int i = 0; i < tokens->list->count; i++) {
		free(tokens->list->items[i]);
	}
	free_SMEList(tokens->list);
	free(tokens);
}

void advance_SMETokens(SMETokens* tokens) {
	if (tokens->tidx + 1 <= tokens->list->count)
		tokens->current = tokens->list->items[tokens->tidx++];
	else
		tokens->current = NULL;
}

int is_digit(const char c) {
	if (c >= 0x30 && c <= 0x39) return 1;
	return 0;
}

SMENode* sme_term(SMETokens* tokens);
SMENode* sme_expr(SMETokens* tokens);


SMENode* sme_factor(SMETokens* tokens) {
	SMEToken* token = tokens->current;
	SMENode* result;

	if (token == NULL) return NULL;

	if (token->type == SMELP) {
		advance_SMETokens(tokens);
		SMENode* result = sme_expr(tokens);
		if (tokens->current->type != SMERP) printf("Missing right parenthesis\n");

		advance_SMETokens(tokens);
		return result;
	} else if (token->type == SMENum){
		advance_SMETokens(tokens);
		result = new_SMENode(SMENum);
		result->value = token->value;
		return result;
	} else if (token->type == SMESub) {
		advance_SMETokens(tokens);
		result = new_SMENode(SMENeg);
		result->left = sme_factor(tokens);
		return result;
	}
	printf("FACTOR EOF OH NO");
}

SMENode* sme_term(SMETokens* tokens) {
	SMENode* factor = sme_factor(tokens);
	SMENode* result = NULL;
	if (tokens->current != NULL){
		if (tokens->current->type == SMEMul) {
			advance_SMETokens(tokens);
			result = new_SMENode(SMEMul);
			result->left = factor;
			result->right = sme_factor(tokens);
		} else if (tokens->current->type == SMEDiv) {
			advance_SMETokens(tokens);
			result = new_SMENode(SMEDiv);
			result->left = factor;
			result->right = sme_factor(tokens);
		}
	} 
	if(result) return result;
	return factor;
}

SMENode* sme_expr(SMETokens* tokens) {
	SMENode* term = sme_term(tokens);
	SMENode* result = NULL;
	if (tokens->current != NULL) {
		if (tokens->current->type == SMEAdd) {
			advance_SMETokens(tokens);
			result = new_SMENode(SMEAdd);
			result->left = term;
			result->right = sme_term(tokens);
		} else if (tokens->current->type == SMESub) {
			advance_SMETokens(tokens);
			result = new_SMENode(SMESub);
			result->left = term;
			result->right = sme_term(tokens);
		}
	}
	return result;
}

double sme_eval(SMENode* node) {
	double res = 0;
	double left = 0;
	double right = 0;
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
	return res;
}

SMETokens* sme_parse(const char* buffer) {
	SMETokens* tokens = new_SMETokens("-23.01 + (2 - (2 + 4)) * 4");
	SMEToken* token = NULL;
	while (tokens->buffer[tokens->idx]) {
		if (is_digit(tokens->buffer[tokens->idx])) {
			while (is_digit(tokens->buffer[tokens->idx])) {
				tokens->temp[tokens->tidx++] = tokens->buffer[tokens->idx++];
				/* In case we have a floating point value keep the dot */
				if (tokens->buffer[tokens->idx] == '.')
					tokens->temp[tokens->tidx++] = tokens->buffer[tokens->idx++];
			}
			tokens->temp[tokens->tidx] = '\0';

			token = new_SMEToken(SMENum);
			token->value = strtod(tokens->temp, NULL);

			append_SMEItem(tokens->list, token);

			tokens->tidx = 0;
		}
		if (tokens->buffer[tokens->idx] == '+') {
			token = new_SMEToken(SMEAdd);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		else if (tokens->buffer[tokens->idx] == '-') {
			token = new_SMEToken(SMESub);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		else if (tokens->buffer[tokens->idx] == '/') {
			token = new_SMEToken(SMEDiv);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		else if (tokens->buffer[tokens->idx] == '*') {
			token = new_SMEToken(SMEMul);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		else if (tokens->buffer[tokens->idx] == '(') {
			token = new_SMEToken(SMELP);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		else if (tokens->buffer[tokens->idx] == ')') {
			token = new_SMEToken(SMERP);
			token->value = strtod(tokens->temp, NULL);
			append_SMEItem(tokens->list, token);
		}
		tokens->idx++;
	}

	tokens->tidx = 0;
}
