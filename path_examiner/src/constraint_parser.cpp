#include <iostream>
#include <regex>
#include <map>
#include <string>
#include <exception>
#include <algorithm>
#include <memory>
#include <cstdio>

#include <yices.h>

#include "constraint_parser.h"


ConstraintParser ConstraintParser::INSTANCE;

ConstraintParser::ConstraintParser()
{
	yices_init();
}

ConstraintParser::~ConstraintParser()
{
	yices_exit();
}

std::ostream& operator<<(std::ostream& out, const Constraint& c)
{
	if (out == std::cout)
		yices_pp_term(stdout, c._inner, 80, 20, 0);
	else if (out == std::cerr)
		yices_pp_term(stderr, c._inner, 80, 20, 0);
	//else discard silently, we cannot do pretty-print elsewhere
#ifdef DEBUG_CONSTRAINTS
	out << c._display;
#endif

	return out;
}


term_t ConstraintParser::getNormalizedVarName(const std::string& var)
{
	auto it = _vars.find(var);
	if (it == _vars.cend())
		it = _vars.emplace(var, yices_new_uninterpreted_term(Yices_int)).first;
	return it->second;
}

term_t ConstraintParser::parse(const std::string& arg)
{
	if (arg[0] == '[') //guards start with '['
		return parseEdge(arg);
	else
		return parseAssign(arg);
}

inline term_t ConstraintParser::parseArgument(const std::string& arg) {
	term_t arg_formula;
	if (!std::regex_match(arg, _numreg)) {
		arg_formula = getNormalizedVarName(arg);
	} else {
		int val = std::stoi(arg);
		arg_formula = val == 0 ? yices_zero() : yices_int64(val);
	}
	return arg_formula;
}

term_t ConstraintParser::parseEdge(const std::string& arg)
{
	auto it = _built.find(arg);
	if (it != _built.cend())
		return it->second;

	std::smatch pieces_match;
	if (!std::regex_match(arg, pieces_match, _edgereg))
		throw std::runtime_error("Could not match " + arg + " against regexp _edgereg, verify input format");

	auto op  = pieces_match[3].str();

	term_t lhs_formula = parseArgument(pieces_match[2].str());
	term_t rhs_formula = parseArgument(pieces_match[4].str());

	op += pieces_match[1].str(); //add the symbol '!' to the operator if present

	return _built.emplace(arg, _ops.at(op)(lhs_formula, rhs_formula)).first->second;
}

term_t ConstraintParser::parseAssign(const std::string& arg)
{
	auto it = _built.find(arg);
	if (it != _built.cend())
		return it->second;

	std::smatch pieces_match;
	if (!std::regex_match(arg, pieces_match, _assignreg))
		throw std::runtime_error("Could not match " + arg + " against regexp _assignreg, verify input format");

	term_t lhs_formula = parseArgument(pieces_match[1].str());
	term_t rhs_formula = parseArgument(pieces_match[2].str());

	return _built.emplace(arg, yices_arith_eq_atom(lhs_formula,rhs_formula)).first->second;
}

Constraint Constraint::conjunct(const std::vector<Constraint>& c)
{
	//TODO: we could check if c.size() < YICES_MAX_ARITY (~2^28)
	// but I don't think we will have a problem here
	std::vector<term_t> terms;
	std::transform(c.cbegin(), c.cend(), std::back_inserter(terms),
			[](const Constraint& c){return c._inner;});
	return Constraint(yices_and(terms.size(), terms.data()));
}

bool Constraint::check_unsatisfiability(const Constraint& c)
{
	auto context_deleter = [](context_t* c) { yices_free_context(c); };
	std::unique_ptr<context_t,decltype(context_deleter)&>
		ctx{yices_new_context(nullptr), context_deleter};

	int code = yices_assert_formula(ctx.get(), c._inner);
	if (code < 0) {
		yices_print_error(stderr);
		throw std::runtime_error("Assert failed on formula");
	}
	switch (yices_check_context(ctx.get(), nullptr)) {
		case STATUS_SAT:
			return false;

		case STATUS_UNSAT:
			return true;

		case STATUS_UNKNOWN:
			return false;

		case STATUS_IDLE:
		case STATUS_SEARCHING:
		case STATUS_INTERRUPTED:
		case STATUS_ERROR:
			yices_print_error(stderr);
			throw("Error in evaluating formula");
	}
}
