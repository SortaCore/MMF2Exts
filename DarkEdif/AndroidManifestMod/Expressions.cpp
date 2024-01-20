#include "Common.hpp"

// None of these can be added from A/C/E menus.
// In theory, only DummyExpression can be added, by manually typing it out.
int Extension::DummyExpression() {
	return 0;
}
void Extension::DummyAction() {
}
bool Extension::DummyCondition() {
	return false;
}
