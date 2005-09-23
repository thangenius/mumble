/* Copyright (C) 2005, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Group.h"
#include <QStack>
#include <QStringList>

Group::Group(Channel *assoc, QString name) {
	c = assoc;
	bInherit = true;
	bInheritable = true;
	qsName = name;
	c->qhGroups[name] = this;
}

QList<int> Group::members() {
	QStack<Group *> s;
	QSet<int> m;
	Channel *p;
	Group *g;
	int i;

	p = c;
	while (p) {
		g = p->qhGroups.value(qsName);
		p = p->cParent;

		if (g) {
			if ((p != c) && ! g->bInheritable)
				break;
			s.push(g);
			if (! g->bInherit)
				break;
		}
	}

	while (! s.isEmpty()) {
		g = s.pop();
		foreach(i, g->qsAdd)
			m.insert(i);
		foreach(i, g->qsRemove)
			m.remove(i);
	}

	return m.toList();
}

QStringList Group::groupNames(Channel *chan) {
	QStack<Channel *> s;
	QSet<QString> m;
	Channel *c = chan;
	Group *g;

	while (c) {
		s.push(c);
		c = c->cParent;
	}

	while (! s.isEmpty()) {
		c = s.pop();
		foreach(g, c->qhGroups) {
			if ((chan != c) && (! g->bInheritable))
				m.remove(g->qsName);
			else
				m.insert(g->qsName);
		}
	}

	return m.toList();
}

bool Group::isMember(Channel *c, QString name, int id) {
	Channel *p;
	Group *g;
	Player *pl = Player::get(id);

	bool m = false;

	if (name.isEmpty())
		return false;

	if (name == "all") {
		return true;
	}

	if (name == "reg") {
		return (id >= 0);
	}

	if (name == "in") {
		if (pl && pl->cChannel == c)
			return true;
		return false;
	}

	if (name == "out") {
		if (pl && pl->cChannel == c)
			return false;
		return true;
	}

	p=c;
	while (p) {
		g = p->qhGroups.value(name);
		p = p->cParent;
		if (g) {
			if ((p != c) && ! g->bInheritable)
				break;
			if (! g->bInherit)
				p = NULL;
			if (g->qsAdd.contains(id))
				m = true;
			if (g->qsRemove.contains(id))
				m = false;
		}
	}
	return m;
}

bool Group::isMember(Channel *c, QString name, Player *p) {
	return isMember(c, name, p->iId);
}

bool Group::isMember(Player *p, QString name) {
	return isMember(p->cChannel, name, p->iId);
}
