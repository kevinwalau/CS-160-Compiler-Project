#include "typecheck.hpp"




std::string default_base_type(BaseType base_type, std::string class_name) {

	switch (base_type) {

	case bt_integer:
		return "Integer";
	case bt_boolean:
		return "Boolean";
	case bt_none:
		return "None";
	case bt_object:
		return class_name;
	default:
		return NULL;

	}
}
CompoundType make_compound_type(BaseType base_Type, std::string object_Class_Name) {
	CompoundType output;
	output.baseType = base_Type;
	output.objectClassName = default_base_type(base_Type, object_Class_Name);
	return output;
}
// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
	switch (code) {
	case undefined_variable:
		std::cerr << "Undefined variable." << std::endl;
		break;
	case undefined_method:
		std::cerr << "Method does not exist." << std::endl;
		break;
	case undefined_class:
		std::cerr << "Class does not exist." << std::endl;
		break;
	case undefined_member:
		std::cerr << "Class member does not exist." << std::endl;
		break;
	case not_object:
		std::cerr << "Variable is not an object." << std::endl;
		break;
	case expression_type_mismatch:
		std::cerr << "Expression types do not match." << std::endl;
		break;
	case argument_number_mismatch:
		std::cerr << "Method called with incorrect number of arguments." << std::endl;
		break;
	case argument_type_mismatch:
		std::cerr << "Method called with argument of incorrect type." << std::endl;
		break;
	case while_predicate_type_mismatch:
		std::cerr << "Predicate of while loop is not boolean." << std::endl;
		break;
	case repeat_predicate_type_mismatch:
		std::cerr << "Predicate of repeat loop is not boolean." << std::endl;
		break;
	case if_predicate_type_mismatch:
		std::cerr << "Predicate of if statement is not boolean." << std::endl;
		break;
	case assignment_type_mismatch:
		std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
		break;
	case return_type_mismatch:
		std::cerr << "Return statement type does not match declared return type." << std::endl;
		break;
	case constructor_returns_type:
		std::cerr << "Class constructor returns a value." << std::endl;
		break;
	case no_main_class:
		std::cerr << "The \"Main\" class was not found." << std::endl;
		break;
	case main_class_members_present:
		std::cerr << "The \"Main\" class has members." << std::endl;
		break;
	case no_main_method:
		std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
		break;
	case main_method_incorrect_signature:
		std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
		break;
	}
	exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

//helper funcs
template <class type_x>
bool typecheck_int(type_x* node) {
	bool flag1 = (node->basetype == bt_integer);
	bool flag2 = (node->objectClassName.compare("Integer"));
	return !flag2 && flag1;
}
template <class type_x>
bool typecheck_bool(type_x* node) {
	bool flag1 = (node->basetype == bt_boolean);
	bool flag2 = (node->objectClassName.compare("Boolean"));
	return flag1 && !flag2;

}

template < class type_x, class type_y>
void expressionSet(type_x *x, type_y *y) {

	x->basetype = y->basetype;
	x->objectClassName = y->objectClassName;
}

bool classCheck(ClassTable *table, std::string &name) {
	bool flag1 = table->count(name) > 0;
	return flag1;
}


template <class type_x>
void  typecheck_AOP(type_x *node) {
	bool flag1 = (typecheck_int(node->expression_1));
	if (!flag1)
		typeError(expression_type_mismatch);
	else {
		checkExprSameType(node->expression_1->basetype, node->expression_1->objectClassName, node->expression_2->basetype, node->expression_2->objectClassName, expression_type_mismatch);

	}
}

bool typecheck_eq(BaseType& a, std::string classObjectN, BaseType& b, std::string& classObject2) {
	
	if (b == a) {
		if (classObjectN.compare(classObject2) != 0)
			return false;
	}
	else if (b != a){
		return false;
	}
	return true;
}

bool typecheck_arg(std::string methodName, MethodTable *mtable, std::list<ExpressionNode*>* ex_list) {

	int listExprNum = (ex_list == NULL) ? 0 : ex_list->size();
	int parameterNum = !(mtable->count(methodName)) ? 0 : mtable->at(methodName).parameters->size();
	if (!(parameterNum == listExprNum))
		typeError(argument_number_mismatch);

	std::list<ExpressionNode*>::iterator iteratorB;
	std::list<CompoundType>::iterator iteratorA;
	if (listExprNum)
		for (iteratorA = mtable->at(methodName).parameters->begin(), iteratorB = ex_list->begin(); iteratorA != (mtable->at(methodName).parameters)->end(); ++iteratorA, iteratorB++)
			if (!typecheck_eq(iteratorA->baseType, iteratorA->objectClassName, (*iteratorB)->basetype, (*iteratorB)->objectClassName))
				typeError(argument_type_mismatch);

	return true;
}

bool checkMethod(MethodCallNode *node, std::string current_class_name, ClassTable &classTable) {
	std::string looking_method(node->identifier_1->name);
	if (node->identifier_2 != NULL) {
		looking_method = node->identifier_2->name;
		if (!classCheck(&classTable, node->identifier_1->objectClassName))
			typeError(not_object);
	}


	do {
		
		if (classTable.at(current_class_name).methods->count(looking_method) <= 0) {
			current_class_name = classTable.at(current_class_name).superClassName;
		}
		else {
			typecheck_arg(looking_method, classTable.at(current_class_name).methods, node->expression_list);
			node->basetype = classTable.at(current_class_name).methods->at(looking_method).returnType.baseType;
			node->objectClassName = classTable.at(current_class_name).methods->at(looking_method).returnType.objectClassName;
			return true;
		}

	} while (!(current_class_name.empty()));

	return false;

}

bool checkClassMems(IdentifierNode *node, std::string current_class_name, ClassTable &classTable) {

	do {
		
		if (classTable.at(current_class_name).members->count(node->name) <= 0) {
			current_class_name = classTable.at(current_class_name).superClassName;
		}
		else {
			node->basetype = classTable.at(current_class_name).members->at(node->name).type.baseType;
			node->objectClassName = classTable.at(current_class_name).members->at(node->name).type.objectClassName;
			return true;
		}

	} while (!current_class_name.empty());

	return false;
}

bool checkLocalVar(IdentifierNode *node, VariableTable *vTable) {
	
	if (vTable->count(node->name) <= 0) {
		return false;
	}
	else {
		node->basetype = vTable->at(node->name).type.baseType;
		node->objectClassName = vTable->at(node->name).type.objectClassName;
		return true;
	}
}

void checkMemMain(ClassNode *node) {


	if ( !(node->declaration_list == NULL) && !node->identifier_1->name.compare("Main"))
		typeError(main_class_members_present);

}
void checkMainExist(ClassNode *node, MethodTable *table) {

	if (!node->identifier_1->name.compare("Main"))
	{
		std::map<std::string, MethodInfo>::iterator iteratorA;
		iteratorA = table->find("main");
		
		if (iteratorA != table->end()) {
			if (iteratorA->second.parameters->size() > 0)
				typeError(main_method_incorrect_signature);
		}
		else{
			typeError(no_main_method);
		}


	}


}
void checkMainClass(ClassTable *table) {

	std::map<std::string, ClassInfo>::iterator iteratorA;
	iteratorA = table->find("Main");
	if (!(iteratorA != table->end()))
		typeError(no_main_class);

}
void checkExprSameType(BaseType& ex1, std::string ex1_objectClassName, BaseType& ex2, std::string ex2_objectClassName, TypeErrorCode errorCode) {
	if (ex2 == ex1) {
		if (ex1_objectClassName.compare(ex2_objectClassName) != 0) {
			typeError(errorCode);
		}
	}
	else {
		typeError(errorCode);
	}
	
}
bool checkVarMember(IdentifierNode *object, IdentifierNode *memberNode, ClassTable &classTable) {
	if (!checkClassMems(memberNode, object->objectClassName, classTable)) {
		typeError(undefined_member);
	}
	if (classTable.at(object->objectClassName).members->count(memberNode->name) > 0) {
		memberNode->basetype = classTable.at(object->objectClassName).members->at(memberNode->name).type.baseType;
		memberNode->objectClassName = classTable.at(object->objectClassName).members->at(memberNode->name).type.objectClassName;
		return true;
	}
	if (!classCheck(&classTable, object->objectClassName)){
		typeError(not_object);
	}
	else{
		return true;
	}
}


//end helper funcs



void TypeCheck::visitProgramNode(ProgramNode* node) {

	classTable = new ClassTable();

	node->visit_children(this);

	checkMainClass(classTable);
}

void TypeCheck::visitClassNode(ClassNode* node) {
	currentVariableTable = NULL;
	currentMethodTable = NULL;
	currentMemberOffset = -4;


	currentClassName = node->identifier_1->name;
	(*classTable)[currentClassName] = *(new ClassInfo);


	classTable->at(currentClassName).methods = new MethodTable();
	classTable->at(currentClassName).members = new VariableTable();

	if (node->identifier_2 != NULL) {
		if (!(classCheck(classTable, node->identifier_2->name))) {
			typeError(undefined_class);
		}
		else {
			classTable->at(currentClassName).superClassName = node->identifier_2->name;

			for (auto iter = classTable->at(node->identifier_2->name).members->begin(); iter != classTable->at(node->identifier_2->name).members->end(); iter++) {
				(*classTable->at(currentClassName).members)[(*iter).first] = { (*iter).second.type, currentMemberOffset += 4, 4 };
			}

		} 
		
	}

	checkMemMain(node);
	node->visit_children(this);
	checkMainExist(node, currentMethodTable);
	classTable->at(currentClassName).membersSize = classTable->at(currentClassName).members->size() * 4;

}

void TypeCheck::visitMethodNode(MethodNode* node) {
	
	currentLocalOffset = 0;
	currentParameterOffset = 8;
	currentMethodTable = classTable->at(currentClassName).methods;
	currentVariableTable = NULL;
	(*currentMethodTable)[node->identifier->name] = *(new methodinfo);
	(*currentMethodTable)[node->identifier->name].parameters = new std::list<CompoundType>();
	currentVariableTable = new VariableTable();
	(*currentMethodTable)[node->identifier->name].returnType = make_compound_type(node->type->basetype, node->type->objectClassName);
	(*currentMethodTable)[node->identifier->name].variables = currentVariableTable;


	expressionSet(node->methodbody, node->type);
	node->visit_children(this);


	if (!(node->parameter_list == NULL))
		for (std::list<ParameterNode*>::iterator iter = node->parameter_list->begin();
		        iter != node->parameter_list->end(); iter++) {
			(*currentMethodTable)[node->identifier->name].parameters->push_back(make_compound_type((*iter)->type->basetype, (*iter)->type->objectClassName));
		}


	std::string lowered_currentClassName(currentClassName);
	lowered_currentClassName[0] = tolower(lowered_currentClassName[0]);
	if (!node->identifier->name.compare(lowered_currentClassName)) {
		if (node->type->basetype != bt_none) {
			
			if (lowered_currentClassName.compare("main") != 0) {
				typeError(constructor_returns_type);
			}
			else {
				typeError(main_method_incorrect_signature);
			} 
		}
	}
	(*currentMethodTable)[node->identifier->name].localsSize = 4 * (*currentMethodTable)[node->identifier->name].variables->size() - 4 * (*currentMethodTable)[node->identifier->name].parameters->size();
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {

	node->visit_children(this);

	
	if (node->returnstatement == NULL) {
		if (node->basetype != bt_none)
			typeError(return_type_mismatch);
	}
	else{
		checkExprSameType(node->basetype, node->objectClassName, node->returnstatement->basetype, node->returnstatement->objectClassName, return_type_mismatch);
	}
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
	
	if (!(classCheck(classTable, node->type->objectClassName) || typecheck_bool(node->type) || typecheck_int(node->type))) {
		typeError(undefined_class);
	}
	else{
		(*currentVariableTable)[node->identifier->name] = { make_compound_type(node->basetype, node->objectClassName), currentParameterOffset += 4, 4 };
	}
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {

	

	
	if (currentVariableTable == NULL){
		if (classCheck(classTable, node->type->objectClassName) || typecheck_int(node->type) || typecheck_bool(node->type)) {
			for (std::list<IdentifierNode*>::iterator itn = node->identifier_list->begin(); itn != node->identifier_list->end(); itn++) {
				(*classTable->at(currentClassName).members)[(*itn)->name] = { make_compound_type(node->basetype, node->objectClassName), currentMemberOffset += 4, 4 };
			}
		} 
		else {
			typeError(undefined_class);
		}
	}
	else {
		
		if (classCheck(classTable, node->type->objectClassName) || typecheck_bool(node->type) || typecheck_int(node->type))
			for (std::list<IdentifierNode*>::iterator itn = node->identifier_list->begin(); itn != node->identifier_list->end(); itn++) {
				(*currentVariableTable)[(*itn)->name] = { make_compound_type(node->type->basetype, node->type->objectClassName), currentLocalOffset -= 4, 4 };
			}
		else
			typeError(undefined_class);
	}

}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
	node->visit_children(this);
	expressionSet(node, node->expression);
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
	node->visit_children(this);
	

	if (node->identifier_2 == NULL) {
		if (checkLocalVar(node->identifier_1, currentVariableTable) ||
		        checkClassMems(node->identifier_1, currentClassName, *classTable))
			checkExprSameType(node->identifier_1->basetype, node->identifier_1->objectClassName, node->expression->basetype, node->expression->objectClassName, assignment_type_mismatch);

		else
			typeError(undefined_variable);
	}

	else {

		if (checkLocalVar(node->identifier_1, currentVariableTable) ||
		        checkClassMems(node->identifier_1, currentClassName, *classTable)) {

			checkVarMember(node->identifier_1, node->identifier_2, *classTable);
			checkExprSameType(node->identifier_2->basetype, node->identifier_2->objectClassName, node->expression->basetype, node->expression->objectClassName, assignment_type_mismatch);
		}
		else
			typeError(undefined_variable);

	}
}

void TypeCheck::visitCallNode(CallNode* node) {
	node->visit_children(this);

}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
	node->visit_children(this);
	if (!(node->expression->basetype == bt_boolean))
		typeError(if_predicate_type_mismatch);
}

void TypeCheck::visitWhileNode(WhileNode* node) {
	node->visit_children(this);
	if (!(node->expression->basetype == bt_boolean))
		typeError(while_predicate_type_mismatch);
}

void TypeCheck::visitRepeatNode(RepeatNode* node) {
	node->visit_children(this);
	if (!(node->expression->basetype == bt_boolean))
		typeError(repeat_predicate_type_mismatch);
}

void TypeCheck::visitPrintNode(PrintNode* node) {
	node->visit_children(this);
	
}

void TypeCheck::visitPlusNode(PlusNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	expressionSet(node, node->expression_1);

}

void TypeCheck::visitMinusNode(MinusNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	expressionSet(node, node->expression_1);

}

void TypeCheck::visitTimesNode(TimesNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	expressionSet(node, node->expression_1);

}

void TypeCheck::visitDivideNode(DivideNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	expressionSet(node, node->expression_1);


}

void TypeCheck::visitLessNode(LessNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	node->basetype = bt_boolean;
	node->objectClassName = "Boolean";
}

void TypeCheck::visitLessEqualNode(LessEqualNode* node) {
	node->visit_children(this);
	typecheck_AOP(node);
	node->basetype = bt_boolean;
	node->objectClassName = "Boolean";
}

void TypeCheck::visitEqualNode(EqualNode* node) {
	node->visit_children(this);
	if (!(typecheck_bool(node->expression_1) || typecheck_int(node->expression_1)))
		typeError(expression_type_mismatch);
	else
		checkExprSameType(node->expression_1->basetype, node->expression_1->objectClassName, node->expression_2->basetype, node->expression_2->objectClassName, expression_type_mismatch);
	
	node->basetype = bt_boolean;
	node->objectClassName = "Boolean";
}

void TypeCheck::visitAndNode(AndNode* node) {
	node->visit_children(this);
	if (!(typecheck_bool(node->expression_1)))
		typeError(expression_type_mismatch);

	else {
		checkExprSameType(node->expression_1->basetype, node->expression_1->objectClassName, node->expression_2->basetype, node->expression_2->objectClassName, expression_type_mismatch);

	}
	expressionSet(node, node->expression_1);
}

void TypeCheck::visitOrNode(OrNode* node) {
	node->visit_children(this);
	if (!(typecheck_bool(node->expression_1)))
		typeError(expression_type_mismatch);

	else {
		checkExprSameType(node->expression_1->basetype, node->expression_1->objectClassName, node->expression_2->basetype, node->expression_2->objectClassName, expression_type_mismatch);

	}
	expressionSet(node, node->expression_1);

}

void TypeCheck::visitNotNode(NotNode* node) {
	node->visit_children(this);
	if (!(typecheck_bool(node->expression))){
		typeError(expression_type_mismatch);
	}
	else{
		expressionSet(node, node->expression);
	}
	

}
void TypeCheck::visitNegationNode(NegationNode* node) {
	node->visit_children(this);
	if (!(typecheck_int(node->expression))) {

		typeError(expression_type_mismatch);
	}
	else
		expressionSet(node, node->expression);
	
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
	node->visit_children(this);
	
	if (node->identifier_2 != NULL) {
		if (checkLocalVar(node->identifier_1, currentVariableTable) ||
		        checkClassMems(node->identifier_1, currentClassName, *classTable)) {
			if (!checkMethod(node, node->identifier_1->objectClassName, *classTable))
				typeError(undefined_method);
		}
		else
			typeError(undefined_variable);
	}
	else
	{
		if (!checkMethod(node, currentClassName, *classTable))
			typeError(undefined_method);

	}
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
	if (!(checkLocalVar(node->identifier_1, currentVariableTable) ||
	        checkClassMems(node->identifier_1, currentClassName, *classTable))) {
		typeError(undefined_variable);
		

	}
	else{
		
		checkVarMember(node->identifier_1, node->identifier_2, *classTable);
		expressionSet(node, node->identifier_2);
	}

}

void TypeCheck::visitVariableNode(VariableNode* node) {
	if (!checkLocalVar(node->identifier, currentVariableTable) && !checkClassMems(node->identifier, currentClassName, *classTable))
		typeError(undefined_variable);
	expressionSet(node, node->identifier);
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
	
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
	
}

void TypeCheck::visitNewNode(NewNode* node) {
	if (!classCheck(classTable, node->identifier->name))
		typeError(undefined_class);
	if (!classTable->at(node->identifier->name).methods->count(node->identifier->name) && node->expression_list != NULL)
		typeError(argument_number_mismatch);
	node->visit_children(this);
	typecheck_arg(node->identifier->name, classTable->at(node->identifier->name).methods, node->expression_list);

}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
	
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
	
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
	
}

void TypeCheck::visitNoneNode(NoneNode* node) {
	
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {


}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
	
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
	std::string string = std::string("");
	for (int i = 0; i < indent; i++)
		string += std::string(" ");
	return string;
}

std::string string(CompoundType type) {
	switch (type.baseType) {
	case bt_integer:
		return std::string("Integer");
	case bt_boolean:
		return std::string("Boolean");
	case bt_none:
		return std::string("None");
	case bt_object:
		return std::string("Object(") + type.objectClassName + std::string(")");
	default:
		return std::string("");
	}
}


void print(VariableTable variableTable, int indent) {
	std::cout << genIndent(indent) << "VariableTable {";
	if (variableTable.size() == 0) {
		std::cout << "}";
		return;
	}
	std::cout << std::endl;
	for (VariableTable::iterator iteratorA = variableTable.begin(); iteratorA != variableTable.end(); iteratorA++) {
		std::cout << genIndent(indent + 2) << iteratorA->first << " -> {" << string(iteratorA->second.type);
		std::cout << ", " << iteratorA->second.offset << ", " << iteratorA->second.size << "}";
		if (iteratorA != --variableTable.end())
			std::cout << ",";
		std::cout << std::endl;
	}
	std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
	std::cout << genIndent(indent) << "MethodTable {";
	if (methodTable.size() == 0) {
		std::cout << "}";
		return;
	}
	std::cout << std::endl;
	for (MethodTable::iterator iteratorA = methodTable.begin(); iteratorA != methodTable.end(); iteratorA++) {
		std::cout << genIndent(indent + 2) << iteratorA->first << " -> {" << std::endl;
		std::cout << genIndent(indent + 4) << string(iteratorA->second.returnType) << "," << std::endl;
		std::cout << genIndent(indent + 4) << iteratorA->second.localsSize << "," << std::endl;
		print(*iteratorA->second.variables, indent + 4);
		std::cout << std::endl;
		std::cout << genIndent(indent + 2) << "}";
		if (iteratorA != --methodTable.end())
			std::cout << ",";
		std::cout << std::endl;
	}
	std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
	std::cout << genIndent(indent) << "ClassTable {" << std::endl;
	for (ClassTable::iterator iteratorA = classTable.begin(); iteratorA != classTable.end(); iteratorA++) {
		std::cout << genIndent(indent + 2) << iteratorA->first << " -> {" << std::endl;
		if (iteratorA->second.superClassName != "")
			std::cout << genIndent(indent + 4) << iteratorA->second.superClassName << "," << std::endl;
		print(*iteratorA->second.members, indent + 4);
		std::cout << "," << std::endl;
		print(*iteratorA->second.methods, indent + 4);
		std::cout << std::endl;
		std::cout << genIndent(indent + 2) << "}";
		if (iteratorA != --classTable.end())
			std::cout << ",";
		std::cout << std::endl;
	}
	std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
	print(classTable, 0);
}
