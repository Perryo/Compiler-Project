
/*
 program → var section body
 var section → id list SEMICOLON                                        -- DONE
 id list → ID COMMA id list | ID                                        -- DONE (ttype not updating issue, used hack)
 body → LBRACE stmt list RBRACE
 stmt list → stmt stmt list | stmt                                      -- DONE
 stmt → assign stmt | print stmt | while stmt | if stmt | switch stmt
 assign stmt → ID EQUAL primary SEMICOLON                               -- DONE
 assign stmt → ID EQUAL expr SEMICOLON                                  -- DONE
 expr → primary op primary                                              -- DONE
 primary → ID | NUM                                                     -- DONE
 op → PLUS | MINUS | MULT | DIV                                         -- DONE
 print stmt → print ID SEMICOLON                                        -- DONE
 while stmt → WHILE condition body
 if stmt → IF condition body
 condition → primary relop primary                                      -- DONE
 relop → GREATER | LESS | NOTEQUAL                                      -- DONE
 switch stmt → SWITCH ID LBRACE case list RBRACE
 switch stmt → SWITCH ID LBRACE case list default case RBRACE
 case list → case case list | case
 case → CASE NUM COLON body
 default case → DEFAULT COLON body
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <istream>
#include <ostream>
#include <sstream>
extern "C" {
#include "compiler.h"
}
#define ALLOC(t) (t*) malloc(sizeof(t))
#define rBRACE '\xFF'

//decl section
AssignmentStatement* assign_stmt();
StatementNode* parse_stmt();
StatementNode* stmt_list();
void expr(AssignmentStatement* assignStmt);
int tokenToInt();
char* safeTokenCopy(char* dest);
PrintStatement* print_stmt();
ValueNode* primary();
ValueNode* getPointerValue(char* value);
int getValue(char* value);
ValueNode* putValueInArray(ValueNode* value);
void id_list();
ValueNode* id_listArray[50];
IfStatement* condition(IfStatement* ifNode);
IfStatement* if_stmt();

void init(){
    int i;
    for(i=0;i<= 50;i++){
        id_listArray[i] = ALLOC(ValueNode);
        id_listArray[i]->value = 0;
    }
  
}

StatementNode* body(){
    getToken();
    StatementNode* stmtNode = ALLOC(StatementNode);
    if(ttype == LBRACE){
        stmtNode = stmt_list();
        getToken();
        if(ttype == RBRACE)
            return stmtNode;
    }
    
    return NULL;
}


//if stmt → IF condition body
IfStatement* case_stmt()//StatementNode* parentNode)
{
    IfStatement* ifNode = ALLOC(IfStatement);
    condition(ifNode);
    if(ttype == COLON)
        getToken();
    ifNode->true_branch = body();
    return ifNode;
}

StatementNode* stmt(){
    StatementNode* stmtNode = ALLOC(StatementNode);
    getToken();
    switch (ttype){
            
        case ID:
            getToken();
            if (ttype == EQUAL)
            {
                ungetToken();
                stmtNode->type = ASSIGN_STMT;
                stmtNode->assign_stmt = assign_stmt(); // call assign statement
                break;
            }
            
        case PRINT:
            stmtNode->type = PRINT_STMT;
            stmtNode->print_stmt = print_stmt();
            break;
           
        case SWITCH:
            stmtNode->type = IF_STMT;
            stmtNode->if_stmt = if_stmt();
            stmtNode->if_stmt->condition_operand2->value = 900000000;
            break;
        
        case CASE:
            stmtNode->type = IF_STMT;
            stmtNode->if_stmt = if_stmt();
            break;
            
        case IF:
            stmtNode->type = IF_STMT;
            stmtNode->if_stmt = if_stmt();
            break;
            
        case WHILE:
            stmtNode->type = IF_STMT;
            stmtNode->if_stmt = if_stmt();
            
            StatementNode* temp;
            StatementNode* goTo = ALLOC(StatementNode);
            goTo->type = GOTO_STMT;
            
            GotoStatement* goToNode = ALLOC(GotoStatement);
            goToNode->target =stmtNode;
            goTo->goto_stmt = goToNode;
            
            temp = stmtNode->if_stmt->true_branch;
            while(temp->next != NULL){
                temp= temp->next; //get last element
            }
            temp->next = goTo;
			temp->next->next = NULL;
            break;
    

    }
    return stmtNode;
}

StatementNode* stmt_list()//StatementNode* stmtNode){
{
    StatementNode* stmtNode = ALLOC(StatementNode);
    StatementNode* stmtListNode = NULL;
    stmtNode = stmt();
    getToken();
    if(ttype == ID || ttype == IF || ttype == PRINT || ttype == WHILE){
        ungetToken();
        stmtListNode = stmt_list();
    }
    if(stmtNode->type == IF_STMT){
        StatementNode* noop = ALLOC(StatementNode);
        noop->type = NOOP_STMT;
        
        StatementNode* temp;
        temp = stmtNode->if_stmt->true_branch;
        if(stmtNode->if_stmt->true_branch != NULL){
        while(temp->next != NULL){
            temp= temp->next; //get last element
        }
        temp->next = noop;
        }
        else{
            temp = noop;
        }
        stmtNode->if_stmt->false_branch = noop;
        stmtNode->next = noop;
        noop->next = stmtListNode;
        
        
        
    }
    else
        stmtNode->next = stmtListNode;
    
    
    if(ttype == RBRACE)
        ungetToken();
    
    return stmtNode;
    
}

StatementNode* parse_generate_intermediate_representation(){
    id_list();
    return body();
}

//if stmt → IF condition body
IfStatement* if_stmt()//StatementNode* parentNode)
{
    IfStatement* ifNode = ALLOC(IfStatement);
    condition(ifNode);
    if(ttype == COLON)
        ttype = getToken();
    ifNode->true_branch = body();
    return ifNode;
}

// condition → primary relop primary
// what is the data structure for this?
IfStatement* condition(IfStatement* ifNode)
{
    ifNode->condition_operand1 = primary();
    if(ttype != NUM)
        ifNode->condition_operand1 = getPointerValue(ifNode->condition_operand1->name);
    getToken();
    
    if(ttype == LBRACE || ttype == COLON){
        ungetToken();
        ifNode->condition_op = LESS;
        ifNode->condition_operand2 = ALLOC(ValueNode);
        ifNode->condition_operand2->value = -9999999;
        return ifNode;
    }
    if(ttype == GREATER || ttype == LESS || ttype == NOTEQUAL){
        switch (ttype){
            case GREATER:   ifNode-> condition_op = GREATER;        break;
            case LESS:      ifNode-> condition_op = LESS;           break;
            case NOTEQUAL:  ifNode-> condition_op = NOTEQUAL;       break;
        }

        //getToken();
    }
    ifNode->condition_operand2= primary();
    if(ttype != NUM)
        ifNode->condition_operand2 = getPointerValue(ifNode->condition_operand2->name);
    return ifNode;
}



//id list → ID COMMA id list | ID
//do i need to store my variables globally and edit them?
void id_list(){
    //getToken();
    int counter = 0;
    while (ttype != SEMICOLON){
         getToken();
        if (ttype == ID){
            ValueNode* value_Node = ALLOC(ValueNode);
			value_Node->value = 0;
            value_Node->name = safeTokenCopy(value_Node->name);
            id_listArray[counter] = value_Node;
            counter++;
        }
       
    }
    return;
}


AssignmentStatement* assign_stmt(){
    AssignmentStatement* assignNode = ALLOC(AssignmentStatement);
    struct ValueNode* newValue = ALLOC(struct ValueNode);
    newValue->name = safeTokenCopy(newValue->name);
    assignNode->left_hand_side = putValueInArray(newValue);
    ttype = getToken();//get equal sign, primary will call again to skip over
    expr(assignNode);
    
    
    return assignNode;
}


//expr → primary op primary
void expr(AssignmentStatement* assignStmt){
    getToken();
    if(ttype == ID || ttype == NUM){
        ungetToken();
        ValueNode* newValue = ALLOC(ValueNode);
        newValue = primary();
        
        if(newValue->name == NULL)//check if this is assignment of value
            assignStmt->operand1 = newValue;
        else
            assignStmt->operand1 = putValueInArray(newValue);
        
        getToken();
    }
    //this is op, only appears once in grammar in exp, no need to isolate
    if (ttype == PLUS || ttype == MINUS || ttype == MULT || ttype == DIV){
        switch (ttype){
            case PLUS:  assignStmt->op = PLUS;      break;
            case MINUS: assignStmt->op = MINUS;     break;
            case MULT:  assignStmt->op = MULT;      break;
            case DIV:   assignStmt->op = DIV;       break;
        }
        getToken();
    }
	else
		assignStmt->op = 0;

    if(ttype == ID || ttype == NUM){
        ungetToken();
        ValueNode* newValue = ALLOC(ValueNode);
        newValue = primary();
        
        if(newValue->name == NULL)//check if this is assignment of value
            assignStmt->operand2 = newValue;
        else
            assignStmt->operand2 = putValueInArray(newValue);
    }
    getToken();
    if(ttype != SEMICOLON)
        ungetToken();
     
    return;
}

//returns a valueNode that fits the grammar of a primary node
//primary → ID | NUM
ValueNode* primary()
{
    getToken();
    if(ttype == ID || ttype == NUM){
        ValueNode* newValue = ALLOC(ValueNode);
        if(ttype == NUM){
			newValue->name = NULL;
            newValue->value = tokenToInt();
		}
        else{
            newValue->name = safeTokenCopy(newValue->name);
        }
        return newValue;
    }
    return NULL;
}

//print stmt → print ID SEMICOLON
PrintStatement* print_stmt(){
    PrintStatement* print = ALLOC(PrintStatement);
    getToken();
    print->id = getPointerValue(token);
    getToken();
        if(ttype == SEMICOLON){
            return print; //valid input, eating up our print tokens, return
    }
    
    return NULL;
}



/*****************************
    Helper Functions
*****************************/

ValueNode* getPointerValue(char* value){
    int i;
    if(value == NULL)
        return NULL;
    for(i=0; i<=50; i++){
        if(id_listArray[i] == NULL){return NULL;}
        else{
            if(strcmp(id_listArray[i]->name, value) == 0)
                return id_listArray[i];
            }
        }
    return NULL;
}

int getValue(char* value){
    int i;
    for(i=0; i<=50; i++){
        if(id_listArray[i] == NULL || value == NULL){return -1;}
        else{
            if(strcmp(id_listArray[i]->name, value) == 0)
                return i;
        }
    }
    return -1;
}

ValueNode* putValueInArray(ValueNode* value){
    int i, inArray;
	if(value == NULL)
		return NULL;
    inArray = getValue(value->name);
    if(inArray == -1){ //if it is not in the array already
        for(i=0; i<=50; i++){
            if(id_listArray[i] == NULL){
                id_listArray[i] = value; //find the end and add it.
                return id_listArray[i];
            }
        }
    }//end if it is not in array
    else{
        if(value->value){ //it has a value, is not a declaration or assignment;
            id_listArray[inArray] = value;//change the entry of that node
        }
        return id_listArray[inArray];
    }
    return id_listArray[inArray];
}


int tokenToInt(){
    std::string tokenString(token);
    std::istringstream converter(tokenString);
    int returnValue = 0;
    converter >> returnValue;
    return returnValue;
}


//Creates a new buffer for the char pointer to prevent access errors
char* safeTokenCopy(char* dest)
{
    char* buffer = new char[strlen(token)+1];
    strcpy(buffer, token);
    return buffer;
}


