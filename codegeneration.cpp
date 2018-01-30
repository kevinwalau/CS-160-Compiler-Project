#include "codegeneration.hpp"




template<typename input>
void codegen(input codeToGen) { std::cout << codeToGen << std::endl; }

template<typename input = std::string, typename... args>
void codegen(input initial, args... remaining) {
	codegen(initial);
	codegen(remaining...);
}

#define startGen codegen(
#define finishGen );



  
// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.

void CodeGenerator::visitProgramNode(ProgramNode* node) {
	// WRITEME: Replace with code if necessary


	startGen
	".data",
	"printstr: .asciz \"%d\\n\"",
	".text",
	".globl " + std::string("Main_main")
	finishGen

	node->visit_children(this);
}
 
void CodeGenerator::visitClassNode(ClassNode* node) {
	currentClassName = node->identifier_1->name;
	currentClassInfo = classTable->at(currentClassName);
	node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
	currentMethodName = node->identifier->name;
	currentMethodInfo = classTable->at(currentClassName).methods->at(currentMethodName);


	std::string methodLabel(currentClassName + "_" + currentMethodName);

	startGen
	methodLabel + ":"
	finishGen

	node->methodbody->accept(this);


}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
	int lSize = currentMethodInfo.localsSize;


	startGen
	"# METHOD STARTUP",
	"push %ebp",
	"mov %esp, %ebp",
	"sub $" + std::to_string(lSize) + ", %esp",
	"push %edi",
	"push %esi",
	"push %ebx",
	"# METHOD STARTUP DONE\n"
	finishGen


	node->visit_children(this);


	if (currentClassName == currentMethodName) {
		startGen
		"mov 8(%ebp), %eax"
		finishGen
	}

	startGen
	"# METHOD CLEANUP",
	"pop %ebx",
	"pop %esi",
	"pop %edi",
	"mov %ebp, %esp", 
	"pop %ebp",
	"ret",   
	"# METHOD CLEANUP DONE\n"
	finishGen
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
	node->visit_children(this);
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
	node->visit_children(this);
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
	node->visit_children(this);
	startGen
	"pop %eax"
	finishGen
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
	node->visit_children(this);

	startGen
	"#### VISIT ASSIGNMENT NODE",
	"pop %eax"
	finishGen

	 
	if (node->identifier_2 != NULL) {
		int memberOffset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;

		int offsetLocal = 0;
		
		if (currentMethodInfo.variables->count(node->identifier_1->name) <= 0) {
			offsetLocal = currentClassInfo.members->at(node->identifier_1->name).offset;

			startGen
			"mov 8(%ebp), %ebx",
			"mov " + std::to_string(offsetLocal) + "(%ebx), %eax",
			"mov %eax, " + std::to_string(memberOffset) + "(%ebx)"
			finishGen
		}
		else {
			offsetLocal = currentMethodInfo.variables->at(node->identifier_1->name).offset;

			startGen
			"mov " + std::to_string(offsetLocal) + "(%ebp), %ebx",
			"mov %eax, " + std::to_string(memberOffset) + "(%ebx)"
			finishGen

		} 


	}

	else {
		int offset;
		
		if (currentMethodInfo.variables->count(node->identifier_1->name) <= 0) {
			offset = currentClassInfo.members->at(node->identifier_1->name).offset;

			int memberOffset = currentClassInfo.members->at(node->identifier_1->name).offset;

			startGen
			"# memberOffset = " + std::to_string(memberOffset),
			"mov 8(%ebp), %ebx",
			"mov %eax, " + std::to_string(memberOffset) + "(%ebx)"
			finishGen
		}
		else {
			offset = currentMethodInfo.variables->at(node->identifier_1->name).offset;

			startGen
			"# " + node->identifier_1->name + " #",
			"# offsetLocal = " + std::to_string(offset),
			"mov %eax, " + std::to_string(offset) + "(%ebp)"
			finishGen
		} 


	}

	startGen
	"#### VISIT ASSIGNMENT NODE DONE \n"
	finishGen
}

void CodeGenerator::visitCallNode(CallNode* node) {
	node->visit_children(this);
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
	node->expression->accept(this);


	std::string labelNum = std::to_string(nextLabel());
	startGen
	"pop %eax",
	"cmp $1, %eax",
	"jne else_label" + labelNum
	finishGen


	auto iter = node->statement_list_1->begin();
	if (node->statement_list_1){
		while (iter != node->statement_list_1->end()) {
			(*iter)->accept(this);
			iter++;

		}

	}

	startGen
	"jmp end_label" + labelNum,
	"else_label" + labelNum + ":"
	finishGen

	if (node->statement_list_2) {
		for (auto iter2 = node->statement_list_2->begin(); iter2 != node->statement_list_2->end(); iter2++) {
			(*iter2)->accept(this);
		}
	}
	

	startGen
	"end_label" + labelNum + ":"
	finishGen
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
	std::string labelNum = std::to_string(nextLabel());

	startGen
	"top_while_label" + labelNum + ":"
	finishGen

	node->expression->accept(this);

	startGen
	"pop %eax",
	"cmp $1, %eax",
	"jne end_while_label" + labelNum
	finishGen



	auto iter = node->statement_list->begin();
	if (node->statement_list){
		while (iter != node->statement_list->end()) {
			(*iter)->accept(this);
			iter++;

		}
	}

	startGen
	"jmp top_while_label" + labelNum,
	"end_while_label" + labelNum + ":"
	finishGen
}

void CodeGenerator::visitRepeatNode(RepeatNode* node) {
	std::string labelNum = std::to_string(nextLabel());

	startGen
	"top_repeat_label" + labelNum + ":"
	finishGen

	auto iter = node->statement_list->begin();
	if (node->statement_list){
		while (iter != node->statement_list->end()) {
			(*iter)->accept(this);
			iter++;

		}
	}

	node->expression->accept(this);

	startGen
	"pop %eax",
	"cmp $1, %eax",
	"jne end_repeat_label" + labelNum
	finishGen

	// Create end label
	startGen
	"jmp top_repeat_label" + labelNum,
	"end_repeat_label" + labelNum + ":"
	finishGen
}

void CodeGenerator::visitPrintNode(PrintNode* node) {



	
	node->visit_children(this);

	startGen

	"push $printstr",
	"call " + std::string("printf"),
	"add $8, %esp"
	finishGen


}

void CodeGenerator::visitPlusNode(PlusNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"add %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
	node->visit_children(this);

	startGen
	"pop %ebx",
	"pop %eax",
	"sub %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"imul %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
	node->visit_children(this);

	startGen
	"pop %ecx",
	"pop %eax",
	"cdq",
	"idiv %ecx",
	"push %eax"
	finishGen
}

void CodeGenerator::visitLessNode(LessNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"cmp %eax, %ebx",
	"setl %dl",
	"movzbl %dl, %eax",
	"push %eax"
	finishGen

}

void CodeGenerator::visitLessEqualNode(LessEqualNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"cmp %eax, %ebx",
	"setle %dl",
	"movzbl %dl, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"cmp %eax, %ebx",
	"sete %dl",
	"movzbl %dl, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitAndNode(AndNode* node) {
	node->visit_children(this);

	startGen 
	"pop %eax",
	"pop %ebx",
	"and %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitOrNode(OrNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"pop %ebx",
	"or %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitNotNode(NotNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"mov $1, %ebx",
	"xor %ebx, %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
	node->visit_children(this);

	startGen
	"pop %eax",
	"neg %eax",
	"push %eax"
	finishGen
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {

	
	startGen
	"#### METHOD CALL START"
	finishGen

	
	int argCount = 0;
	if (node->expression_list != NULL ) {
		for (auto iter = node->expression_list->rbegin(); iter != node-> expression_list -> rend(); iter++) {
			(*iter) -> accept(this);
			argCount++;
		}
	}

	std::string methName(node->identifier_1->name);
	std::string class_name(currentClassName);



	if (node->identifier_2 != NULL) {
		class_name = node->identifier_1->objectClassName;
		methName = node->identifier_2->name;

		
		if (currentMethodInfo.variables->count(node->identifier_1->name) <= 0) {
			startGen
			"mov 8(%ebp), %ebx",
			"push " + std::to_string(currentClassInfo.members->at(node->identifier_1->name).offset) + "(%ebx)"
			finishGen
		}
		else {
			startGen
			"push " + std::to_string(currentMethodInfo.variables->at(node->identifier_1->name).offset) + "(%ebp)"
			finishGen
		} 

		while (classTable->at(class_name).methods->count(methName) == 0) {
			class_name = classTable->at(class_name).superClassName;
		}
	}

	else {
		while (classTable->at(class_name).methods->count(methName) == 0) {
			class_name = classTable->at(class_name).superClassName;
		}

		startGen
		"push 8(%ebp)"
		finishGen
	} 
	

	startGen
	"call " + class_name + "_" + methName,
	"add $" + std::to_string(argCount * 4 + 4) + ", %esp",
	"push %eax",
	"#### METHOD CALL END\n"
	finishGen
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
	node->visit_children(this);

	startGen
	"#### VISIT MEMBER ACCESS"
	finishGen

	int memberOffset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;

	
	if (currentMethodInfo.variables->count(node->identifier_1->name) <= 0)  {
		int offsetLocal = currentClassInfo.members->at(node->identifier_1->name).offset;
		startGen
		"# offsetLocal = " + std::to_string(offsetLocal),
		"# memebrOffset = " + std::to_string(memberOffset),
		"mov 8(%ebp), %ebx",

		"mov " + std::to_string(offsetLocal) + "(%ebx), %ecx",
		"mov " + std::to_string(memberOffset) + "(%ecx), %eax",
		"push %eax"
		finishGen

	}
	else {
		int offsetLocal = currentMethodInfo.variables->at(node->identifier_1->name).offset;

		startGen
		"# offsetLocal = " + std::to_string(offsetLocal),
		"# memebrOffset = " + std::to_string(memberOffset),
		"mov " + std::to_string(offsetLocal) + "(%ebp), %ebx",
		"mov " + std::to_string(memberOffset) + "(%ebx), %eax",
		"push %eax"
		finishGen
	} 


 
	startGen
	"#### VISIT MEMBER ACCESS DONE\n"
	finishGen
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
	node->visit_children(this);
	startGen
	"# VISIT VARIABLE"
	finishGen

	if (currentClassInfo.members->count(node->identifier->name) > 0) {
		int offset = currentClassInfo.members->at(node->identifier->name).offset;

		startGen
		"#  VARIABLE MEMBER",
		"mov 8(%ebp), %ebx",
		"mov " + std::to_string(offset) + "(%ebx), %eax",
		"push %eax"
		finishGen
	}

	else if (currentMethodInfo.variables->count(node->identifier->name) > 0) {
		int offset = currentMethodInfo.variables->at(node->identifier->name).offset;

		startGen
		"#  VARIABLE LOCAL",
		"push " + std::to_string(offset) + "(%ebp)"
		finishGen


	} 

	startGen
	"# VISIT VARIABLE DONE\n"
	finishGen
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
	node->visit_children(this);

	startGen
	"push $" + std::to_string(node->integer->value)
	finishGen
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
	node->visit_children(this);
	startGen
	"push $" + std::to_string(node->integer->value)
	finishGen
}

void CodeGenerator::visitNewNode(NewNode* node) {
	node->visit_children(this);

	std::string mallocCall("malloc");


	int classSize = classTable->at(node->identifier->name).membersSize;


	startGen
	"# name = " + node->identifier->name,
	"# classSize = " + std::to_string(classSize)
	finishGen

	if (classTable->at(node->identifier->name).methods->count(node->identifier->name) <= 0) {
		startGen
		"push $" + std::to_string(classSize),
		"call " + mallocCall,
		"add $4, %esp",
		"push %eax"
		finishGen
	}
	else {
		startGen
		"#### CONSTRUCTOR CALL START"
		finishGen


		
		int argCount = 0;
		if (node->expression_list != NULL ) {
			for (auto iter = node->expression_list->rbegin(); iter != node-> expression_list -> rend(); iter++) {
				(*iter) -> accept(this);
				argCount++;
			}
		}

		startGen
		"push $" + std::to_string(classSize),
		"call " + mallocCall,
		"add $4, %esp",
		"push %eax",
		"call " + node->identifier->name + "_" + node->identifier->name,
		"add $" + std::to_string(argCount * 4 + 4) + ", %esp",
		"push %eax",
		"#### CONSTRUCTOR CALL END\n"
		finishGen
	} 




}

void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode * node) {
	node->visit_children(this);
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode * node) {
	node->visit_children(this);
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode * node) {
	node->visit_children(this);
}

void CodeGenerator::visitNoneNode(NoneNode * node) {
	node->visit_children(this);
}

void CodeGenerator::visitIdentifierNode(IdentifierNode * node) {
	node->visit_children(this);
}

void CodeGenerator::visitIntegerNode(IntegerNode * node) {
	node->visit_children(this);
}
