/* The code in this file is by TurboFerret, from his TurboTemplate (modified to use my rVector class) */

#ifndef OOP_EXT
#define OOP_EXT

inline LPCONDITION * getConditions() {
	LPCONDITION * c=new LPCONDITION[Conditions.size()+1];
	size_t i=0;
	while (i!=Conditions.size()) {
		c[i]=Conditions[i]->getCondition();
		++i;
	}
	c[Conditions.size()]=0;
	return c;
}

inline LPACTION * getActions() {
	LPACTION * a=new LPACTION[Actions.size()+1];
	size_t i=0;
	while (i!=Actions.size()) {
		a[i]=Actions[i]->getAction();
		++i;
	}
	a[Actions.size()]=0;
	return a;
}

inline LPEXPRESSION * getExpressions() {
	LPEXPRESSION * e=new LPEXPRESSION[Expressions.size()+1];
	size_t i=0;
	while (i!=Expressions.size()) {
		e[i]=Expressions[i]->getExpression();
		++i;
	}
	e[Expressions.size()]=0;
	return e;
}

inline short * getConditionInfos() {
	rVector<short> infos;
	size_t i=0;
	while (i!=Conditions.size()) {
		infos.push_back(CONDITION_ID(i));
		infos.push_back(CONDITION_ID(i));
		infos.push_back(i);
		infos.push_back(Conditions[i]->getFlags());
		infos.push_back(Conditions[i]->getParamCount());
		size_t pi;
		for (pi=0;pi<Conditions[i]->getParamCount();pi++)
			infos.push_back(Conditions[i]->getParamType(pi));
		for (pi=0;pi<Conditions[i]->getParamCount();pi++)
			infos.push_back(CONDITION_ID(i)+1+pi);
		++i;
	}
	short * ret = new short[infos.size()];
	for (i=0;i<infos.size();i++)
		ret[i]=infos[i];
	return ret;
}

inline short * getActionInfos() {
	rVector<short> infos;
	size_t i=0;
	while (i!=Actions.size()) {
		infos.push_back(ACTION_ID(i));
		infos.push_back(ACTION_ID(i));
		infos.push_back(i);
		infos.push_back(Actions[i]->getFlags());
		infos.push_back(Actions[i]->getParamCount());
		size_t pi;
		for (pi=0;pi<Actions[i]->getParamCount();pi++)
			infos.push_back(Actions[i]->getParamType(pi));
		for (pi=0;pi<Actions[i]->getParamCount();pi++)
			infos.push_back(ACTION_ID(i)+1+pi);
		++i;
	}
	short * ret = new short[infos.size()];
	for (i=0;i<infos.size();i++)
		ret[i]=infos[i];
	return ret;
}

inline short * getExpressionInfos() {
	rVector<short> infos;
	size_t i=0;
	while (i!=Expressions.size()) {
		infos.push_back(EXPRESSION_ID(i));
		infos.push_back(EXPRESSION_ID(i));
		infos.push_back(i);
		infos.push_back(Expressions[i]->getFlags());
		infos.push_back(Expressions[i]->getParamCount());
		size_t pi;
		for (pi=0;pi<Expressions[i]->getParamCount();pi++)
			infos.push_back(Expressions[i]->getParamType(pi));
		for (pi=0;pi<Expressions[i]->getParamCount();pi++)
			infos.push_back(0);
		++i;
	}
	short * ret = new short[infos.size()];
	for (i=0;i<infos.size();i++)
		ret[i]=infos[i];
	return ret;
}

#endif // !OOP_EXT