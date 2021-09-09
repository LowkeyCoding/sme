# sme
Simple math evaluation

# Build
To run tests, run `make tests` then run `./test` 
```
Runing 4 tests:

1) TOKENIZER: Passed
2) PARSER:    Passed
3) EVALUATOR: Passed
4) VARIABLES: Passed

Runs: 4 Passes: 4 Fails: 0
```
To run repl, run `make repl` then run `./repl`
```
Simple math evaluator
To add variable type (:name=value)
To quit type (quit)

sme>
```
# Usage

## Just calculate some math
To do a simple calculation, you only need to call the `sme_calc(char*, SMEList*)` function.
```c
double res = sme_calc("2+3", NULL);
```

## Use variables
To use variables, first create an instance of `SMElist*` then append a variable to the list use `append_SMEItem(SMElist*, SMEVar*)`. To create a variable use `new_SMEVar(char*, double)`.
```c
SMEList* vars = new_SMEList();
append_SMEItem(vars, new_SMEVar("a", 3.4));
append_SMEItem(vars, new_SMEVar("b", 4.2069));
double res = sme_calc("2 + 3 * a + b", vars);
```

## Access toknens and nodes
To get access to the tokens and nodes, you will first have to create an instance of tokenizer using `new_SMETokenizer(char*, SMEList*)`. That will then generate a list of tokens as well, then to get the nodes run `sme_parse(SMETokenizer*)`. Lastly, to run the calculation pass, the `root` node to `sme_eval`
```c
SMEList* vars = new_SMEList();
append_SMEItem(vars, new_SMEVar("a", 3.4));

SMETokenizer* tokenizer = new_SMETokenizer("2+3 * a", vars);
SMENode* root = sme_parse(tokenizer);

double res = sme_eval(root);

free_SMETokenizer(tokenizer);
free_SMENode(root);
```

# Operators

* Binary
  * `+`
  * `-`
  * `*`
  * `/`
* unary
  * `-` Negates number.
  * `+` Makes the result positive if it is negative.
  * `floor` Rounds down number.
  * `ceil` Rounds up number.
* Other
  * `(` Starts a collection.
  * `)` Ends a collection.
