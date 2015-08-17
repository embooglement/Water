#include <cmath>
#include <stdexcept>
#include "astnode.h"
#include "runtime_errors.h"
#include "iohelpers.h"

using namespace std;

/* ===== ASTNode ===== */

ASTNode::ASTNode(const TokenMetaData& meta, shared_ptr<Scope> scope)
	: _meta(meta), _scope(scope) {
		if (!_scope) {
			throw invalid_argument("scope cannot be null");
		}
	}

const TokenMetaData& ASTNode::meta() const {
	return _meta;
}

shared_ptr<Scope> ASTNode::scope() const {
	return _scope;
}

bool ASTNode::isLValue() const {
	return false;
}

bool ASTNode::isConst(const std::shared_ptr<Scope>& scope) const {
	return false;
}

shared_ptr<Value> ASTNode::evaluate() const {
	throw InterpretorError("evaluate not implemented");
}

void ASTNode::assign(shared_ptr<Value> rhs) const {
	throw InterpretorError("(not assignable)");
}

/* ===== IdentifierNode ===== */

IdentifierNode::IdentifierNode(const TokenMetaData& meta, shared_ptr<Scope> scope, string identifier)
	: ASTNode(meta, move(scope)), _identifier(move(identifier)) {}

bool IdentifierNode::isLValue() const {
	return true;
}

bool IdentifierNode::isConst(const std::shared_ptr<Scope>& scope) const {
	return scope->getInfo(_identifier)->is_const;
}

void IdentifierNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << _identifier;
}

shared_ptr<Value> IdentifierNode::evaluate() const {
	return scope()->getValue(_identifier);
}

void IdentifierNode::assign(shared_ptr<Value> rhs) const {
	scope()->setValue(str(), move(rhs));
}

const string& IdentifierNode::str() const {
	return _identifier;
}

/* ===== NumberLiteralNode ===== */

NumberLiteralNode::NumberLiteralNode(const TokenMetaData& meta, shared_ptr<Scope> scope, string number)
	: ASTNode(meta, move(scope)), _number(stod(move(number))) {}

void NumberLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << _number;
}

shared_ptr<Value> NumberLiteralNode::evaluate() const {
	return NumberValue::create(_number);
}

/* ===== StringLiteralNode ===== */

StringLiteralNode::StringLiteralNode(const TokenMetaData& meta, shared_ptr<Scope> scope, string str)
	: ASTNode(meta, move(scope)), _str(move(str)) {}

void StringLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "\"" << _str << "\"";
}

shared_ptr<Value> StringLiteralNode::evaluate() const {
	return StringValue::create(_str);
}

/* ===== BooleanLiteralNode ===== */

BooleanLiteralNode::BooleanLiteralNode(const TokenMetaData& meta, shared_ptr<Scope> scope, bool boolean)
	: ASTNode(meta, move(scope)), _boolean(boolean) {}

void BooleanLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << (_boolean ? "true" : "false");
}

shared_ptr<Value> BooleanLiteralNode::evaluate() const {
	return BooleanValue::create(_boolean);
}

/* ===== NullLiteralNode ===== */

NullLiteralNode::NullLiteralNode(const TokenMetaData& meta, shared_ptr<Scope> scope)
	: ASTNode(meta, move(scope)) {}

void NullLiteralNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "null";
}

shared_ptr<Value> NullLiteralNode::evaluate() const {
	return NullValue::get();
}

/* ===== ArrayLiteralNode ===== */

ArrayLiteralNode::ArrayLiteralNode(const TokenMetaData& meta, shared_ptr<Scope> scope, vector<shared_ptr<ASTNode>> elements)
	: ASTNode(meta, move(scope)), _elements(move(elements)) {}

void ArrayLiteralNode::output(ostream& out, int indent) const {
	if (_elements.empty()) {
		out << io::indent(indent) << "(array 0)";
		return;
	}

	out << io::indent(indent) << "(array " << _elements.size() << endl;

	for (auto&& element : _elements) {
		element->output(out, indent + 1);
		out << endl;
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> ArrayLiteralNode::evaluate() const {
	vector<shared_ptr<Value>> elements;

	for (auto&& element_node : _elements) {
		elements.push_back(element_node->evaluate());
	}

	return make_shared<ArrayValue>(move(elements));
}

/* ===== SubscriptNode ===== */

SubscriptNode::SubscriptNode(const TokenMetaData& meta, shared_ptr<Scope> scope, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> index)
	: ASTNode(meta, move(scope)), _lhs(move(lhs)), _index(move(index)) {}

void SubscriptNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(subscript" << endl;
	_lhs->output(out, indent + 1);
	out << endl;
	_index->output(out, indent + 1);
	out << endl << io::indent(indent) << ")";
}

shared_ptr<Value> SubscriptNode::evaluate() const {
	auto lhs = _lhs->evaluate();
	if (lhs->type() != ValueType::Array) {
		throw TypeError("Expression is not of type Array");
	}

	auto arr = static_pointer_cast<ArrayValue>(lhs);
	return arr->get(_index->evaluate());
}

bool SubscriptNode::isLValue() const {
	return _lhs->isLValue();
}

bool SubscriptNode::isConst(const std::shared_ptr<Scope>& scope) const {
	return _lhs->isConst(scope);
}

void SubscriptNode::assign(shared_ptr<Value> rhs) const {
	auto lhs = _lhs->evaluate();
	lhs->set(_index->evaluate(), move(rhs));
}

/* ===== AccessMemberNode ===== */

AccessMemberNode::AccessMemberNode(const TokenMetaData& meta, shared_ptr<Scope> scope, std::shared_ptr<ASTNode> lhs, std::string member)
	: ASTNode(meta, move(scope)), _lhs(move(lhs)), _member(make_shared<StringValue>(move(member))) {}

bool AccessMemberNode::isLValue() const {
	return _lhs->isLValue();
}

bool AccessMemberNode::isConst(const std::shared_ptr<Scope>& scope) const {
	return _lhs->isConst(scope);
}

void AccessMemberNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(access" << endl;

	_lhs->output(out, indent + 1);
	out << endl << io::indent(indent + 1) << _member << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> AccessMemberNode::evaluate() const {
	auto lhs = _lhs->evaluate();
	return lhs->get(_member);
}

void AccessMemberNode::assign(shared_ptr<Value> rhs) const {
	auto lhs = _lhs->evaluate();
	lhs->set(_member, move(rhs));
}

/* ===== BinaryOperatorNode ===== */

BinaryOperatorNode::BinaryOperatorNode(const TokenMetaData& meta, shared_ptr<Scope> scope, Builtin op, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
	: ASTNode(meta, move(scope)), _op(op), _left(move(left)), _right(move(right)) {}

void BinaryOperatorNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(" << getBuiltinString(_op) << "\n";

	_left->output(out, indent + 1);
	out << "\n";
	_right->output(out, indent + 1);
	out << "\n";

	out << io::indent(indent) << ")";
}

shared_ptr<Value> BinaryOperatorNode::evaluate() const {
	// Short circuit evaluate logical operators
	switch (_op) {
		case Builtin::LogicalAnd:
			if (toBoolean(_left->evaluate())) {
				return BooleanValue::create(toBoolean(_right->evaluate()));
			} else {
				return BooleanValue::create(false);
			}
		case Builtin::LogicalOr:
			if (toBoolean(_left->evaluate())) {
				return BooleanValue::create(true);
			} else {
				return BooleanValue::create(toBoolean(_right->evaluate()));
			}
		default:
			break;
	}

	auto add = [](double x, double y) { return x + y; };
	auto subtract = [](double x, double y) { return x - y; };
	auto multiply = [](double x, double y) { return x * y; };
	auto divide = [](double x, double y) { return x / y; };

	auto lhs = _left->evaluate();
	auto rhs = _right->evaluate();

	switch (_op) {
		// Assignments
		case Builtin::Assignment:
			_left->assign(move(rhs));
			break;
		case Builtin::AdditionAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, add));
			break;
		case Builtin::SubtractionAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, subtract));
			break;
		case Builtin::MultiplicationAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, multiply));
			break;
		case Builtin::DivisionAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, divide));
			break;
		case Builtin::ModulusAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, fmodl));
			break;
		case Builtin::ExponentAssignment:
			_left->assign(NumberValue::applyOperator(lhs, rhs, powl));
			break;

		// Arithmetic
		case Builtin::Addition:
			return NumberValue::applyOperator(lhs, rhs, add);
		case Builtin::Subtraction:
			return NumberValue::applyOperator(lhs, rhs, subtract);
		case Builtin::Multiplication:
			return NumberValue::applyOperator(lhs, rhs, multiply);
		case Builtin::Division:
			return NumberValue::applyOperator(lhs, rhs, divide);
		case Builtin::Modulus:
			return NumberValue::applyOperator(lhs, rhs, fmodl);
		case Builtin::Exponent:
			return NumberValue::applyOperator(lhs, rhs, powl);

		// Comparisons
		case Builtin::LessThan:
			return BooleanValue::create(toNumber(lhs) < toNumber(rhs));
		case Builtin::LessThanOrEqual:
			return BooleanValue::create(toNumber(lhs) <= toNumber(rhs));
		case Builtin::GreaterThan:
			return BooleanValue::create(toNumber(lhs) > toNumber(rhs));
		case Builtin::GreaterThanOrEqual:
			return BooleanValue::create(toNumber(lhs) >= toNumber(rhs));
		case Builtin::EqualTo:
			return BooleanValue::create(toNumber(lhs) == toNumber(rhs));
		case Builtin::NotEqualTo:
			return BooleanValue::create(toNumber(lhs) != toNumber(rhs));

		default:
			throw InterpretorError("operator not implemented");
	}

	return NullValue::get();
}

/* ===== UnaryOperatorNode ===== */

UnaryOperatorNode::UnaryOperatorNode(const TokenMetaData& meta, shared_ptr<Scope> scope, Builtin op, shared_ptr<ASTNode> expr)
	: ASTNode(meta, move(scope)), _op(op), _expr(move(expr)) {}

void UnaryOperatorNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(" << getBuiltinString(_op) << "\n";

	_expr->output(out, indent + 1);
	out << "\n";

	out << io::indent(indent) << ")";
}

shared_ptr<Value> UnaryOperatorNode::evaluate() const {
	auto expr = _expr->evaluate();

	switch (_op) {
		// Arithmetic
		case Builtin::Negation:
			return NumberValue::create(-toNumber(expr));

		// Logical
		case Builtin::LogicalNot:
			return BooleanValue::create(!toBoolean(expr));

		default:
			throw InterpretorError("operator not implemented");
	}
}

/* ===== FunctionCallNode ===== */

FunctionCallNode::FunctionCallNode(const TokenMetaData& meta, shared_ptr<Scope> scope, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode>> arguments)
	: ASTNode(meta, move(scope)), _caller(move(caller)), _arguments(move(arguments)) {}

void FunctionCallNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(";

	auto identifier = dynamic_pointer_cast<IdentifierNode>(_caller);
	if (identifier) {
		identifier->output(out, 0);
	} else {
		out << "function call" << endl;
		_caller->output(out, indent + 1);
	}

	out << endl;

	for (auto&& argument : _arguments) {
		argument->output(out, indent + 1);
		out << "\n";
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> FunctionCallNode::evaluate() const {
	auto caller = _caller->evaluate();
	if (caller->type() != ValueType::Function) {
		throw TypeError("Expression is not of type Function");
	}

	auto func = static_pointer_cast<FunctionValue>(caller);
	vector<shared_ptr<Value>> arguments;

	for (auto&& argument_node : _arguments) {
		arguments.push_back(argument_node->evaluate());
	}

	return func->call(move(arguments));
}

/* ===== BlockNode ===== */

BlockNode::BlockNode(const TokenMetaData& meta, shared_ptr<Scope> scope, bool is_new_scope, vector<shared_ptr<ASTNode>> statements)
	: ASTNode(meta, move(scope)), _is_new_scope(is_new_scope), _statements(move(statements)) {}

bool BlockNode::isNewScope() const {
	return _is_new_scope;
}

void BlockNode::output(ostream& out, int indent) const {
	out << io::indent(indent);

	if (isNewScope()) {
		out << "(block" << endl;
	} else {
		out << "(global scope" << endl;
	}

	for (auto&& statement : _statements) {
		statement->output(out, indent + 1);
		out << endl;
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> BlockNode::evaluate() const {
	for (auto&& statement : _statements) {
		auto val = statement->evaluate();

		if (val && val->type() == ValueType::Sentinel) {
			return val;
		}
	}

	return nullptr;
}

/* ===== IfStatementNode ===== */

IfStatementNode::IfStatementNode(const TokenMetaData& meta, shared_ptr<Scope> scope, shared_ptr<ASTNode> condition, shared_ptr<ASTNode> then_statement, shared_ptr<ASTNode> else_statement)
	: ASTNode(meta, move(scope)), _condition(move(condition)), _then(move(then_statement)), _else(move(else_statement)) {}

void IfStatementNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(if" << endl;

	{
		out << io::indent(indent + 1) << "(condition" << endl;
		_condition->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	{
		out << io::indent(indent + 1) << "(then" << endl;
		_then->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	if (_else) {
		out << io::indent(indent + 1) << "(else" << endl;
		_else->output(out, indent + 2);
		out << endl;
		out << io::indent(indent + 1) << ")" << endl;
	}

	out << io::indent(indent) << ")";
}

shared_ptr<Value> IfStatementNode::evaluate() const {
	auto condition = _condition->evaluate();
	if (!condition) {
		throw InterpretorError("condition is null");
	}

	if (condition->type() != ValueType::Boolean) {
		throw TypeError("Condition is not of type Boolean");
	}

	bool condition_value = static_pointer_cast<BooleanValue>(condition)->valueOf();
	if (condition_value) {
		return _then->evaluate();
	} else if (_else) {
		return _else->evaluate();
	}

	return nullptr;
}

/* ===== WhileStatementNode ===== */

WhileStatementNode::WhileStatementNode(const TokenMetaData& meta, shared_ptr<Scope> scope, std::shared_ptr<ASTNode> condition, std::shared_ptr<ASTNode> loop)
	: ASTNode(meta, move(scope)), _condition(move(condition)), _loop(move(loop)) {}

void WhileStatementNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(while" << endl;

	{
		out << io::indent(indent + 1) << "(condition" << endl;
		_condition->output(out, indent + 2);
		out << endl << io::indent(indent + 1) << ")";
	}

	out << endl;

	{
		out << io::indent(indent + 1) << "(loop" << endl;
		_loop->output(out, indent + 2);
		out << endl << io::indent(indent + 1) << ")";
	}

	out << endl << io::indent(indent) << ")";
}

shared_ptr<Value> WhileStatementNode::evaluate() const {
	while (true) {
		auto condition = _condition->evaluate();

		if (!condition) {
			throw InterpretorError("condition is null");
		}

		if (condition->type() != ValueType::Boolean) {
			throw TypeError("Condition is not of type Boolean");
		}

		bool condition_value = static_pointer_cast<BooleanValue>(condition)->valueOf();
		if (!condition_value) {
			break;
		}

		auto val = _loop->evaluate();

		if (val && val->type() == ValueType::Sentinel) {
			auto sentinel = static_pointer_cast<SentinelValue>(val);
			if (sentinel->isBreak()) {
				break;
			} else if (sentinel->isContinue()) {
				continue;
			}
		}
	}

	return NullValue::get();
}

/* ===== DeclarationNode ===== */

DeclarationNode::DeclarationNode(const TokenMetaData& meta, shared_ptr<Scope> scope, bool is_const, string identifier, shared_ptr<ASTNode> expr)
	: ASTNode(meta, move(scope)), _is_const(is_const), _identifier(move(identifier)), _expr(move(expr)) {}

void DeclarationNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(decl ";

	if (_is_const) {
		out << "const ";
	}

	out << _identifier << endl;

	_expr->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> DeclarationNode::evaluate() const {
	if (_expr) {
		scope()->setValue(_identifier, _expr->evaluate());
	}

	return nullptr;
}

/* ===== FunctionDeclarationNode ===== */

FunctionDeclarationNode::FunctionDeclarationNode(const TokenMetaData& meta, shared_ptr<Scope> scope, string identifier, vector<string> argument_names, shared_ptr<ASTNode> body)
	: ASTNode(meta, move(scope)), _identifier(move(identifier)), _argument_names(move(argument_names)), _body(move(body)) {}

void FunctionDeclarationNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(decl func";

	if (!_identifier.empty()) {
		out << " " << _identifier;
	}

	out << endl << io::indent(indent + 1) << "(";

	int arguments_count = _argument_names.size();

	for (int i = 0; i < arguments_count; ++i) {
		out << _argument_names[i];

		if (i + 1 < arguments_count) {
			out << " ";
		}
	}

	out << ")" << endl;

	_body->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> FunctionDeclarationNode::evaluate() const {
	return UserDefinedFunctionValue::create(_identifier, _argument_names, _body);
}

/* ===== ReturnNode ===== */

ReturnNode::ReturnNode(const TokenMetaData& meta, shared_ptr<Scope> scope, shared_ptr<ASTNode> expr)
	: ASTNode(meta, move(scope)), _expr(move(expr)) {}

void ReturnNode::output(ostream& out, int indent) const {
	if (!_expr) {
		out << io::indent(indent) << "(return)";
		return;
	}

	out << io::indent(indent) << "(return" << endl;

	_expr->output(out, indent + 1);
	out << endl;

	out << io::indent(indent) << ")";
}

shared_ptr<Value> ReturnNode::evaluate() const {
	if (_expr) {
		auto val = _expr->evaluate();

		if (!val) {
			throw InterpretorError("No value to return");
		}

		if (val->type() == ValueType::Sentinel) {
			throw InterpretorError("Returning sentinel value");
		}

		scope()->setValue(return_value_alias, move(val));
	}

	return SentinelValue::Return;
}

/* ===== BreakNode ===== */

BreakNode::BreakNode(const TokenMetaData& meta, shared_ptr<Scope> scope)
	: ASTNode(meta, move(scope)) {}

void BreakNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(break)";
}

shared_ptr<Value> BreakNode::evaluate() const {
	return SentinelValue::Break;
}

/* ===== ContinueNode ===== */

ContinueNode::ContinueNode(const TokenMetaData& meta, shared_ptr<Scope> scope)
	: ASTNode(meta, move(scope)) {}

void ContinueNode::output(ostream& out, int indent) const {
	out << io::indent(indent) << "(continue)";
}

shared_ptr<Value> ContinueNode::evaluate() const {
	return SentinelValue::Continue;
}
