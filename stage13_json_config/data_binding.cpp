#include "data_binding.h"
#include "sim_data_manager.h"
#include "config_parser.h"
#include "tank_widget.h"
#include "pump_widget.h"
#include "valve_widget.h"
#include "pipe_widget.h"
#include "value_display.h"
#include <QDebug>
#include <QtMath>

// ── ExprEval ─────────────────────────────────────────────────────

ExprEval::ExprEval(const QString &expr, qreal val) : m_expr(expr), m_val(val)
{
    tokenize();
}

qreal ExprEval::evaluate(const QString &expr, qreal val)
{
    if (expr.trimmed().isEmpty()) return val;
    ExprEval ev(expr, val);
    return ev.parse();
}

void ExprEval::tokenize()
{
    m_tokens.clear();
    QString s = m_expr.trimmed();
    int i = 0;

    while (i < s.size()) {
        QChar c = s[i];

        if (c.isSpace()) { i++; continue; }

        if (c.isDigit() || c == '.') {
            int start = i;
            while (i < s.size() && (s[i].isDigit() || s[i] == '.')) i++;
            Token t; t.type = TNUM; t.numVal = s.mid(start, i - start).toDouble();
            m_tokens.append(t);
            continue;
        }

        if (c.isLetter()) {
            int start = i;
            while (i < s.size() && s[i].isLetter()) i++;
            QString id = s.mid(start, i - start);
            if (id == "val") {
                Token t; t.type = TVAL; m_tokens.append(t);
            } else {
                qWarning() << "ExprEval: unknown identifier" << id;
                Token t; t.type = TERR; m_tokens.append(t);
            }
            continue;
        }

        Token t;
        if (c == '+')      { t.type = TPLUS; i++; }
        else if (c == '-') { t.type = TMINUS; i++; }
        else if (c == '*') { t.type = TSTAR; i++; }
        else if (c == '/') { t.type = TSLASH; i++; }
        else if (c == '(') { t.type = TLPAREN; i++; }
        else if (c == ')') { t.type = TRPAREN; i++; }
        else if (c == '!') {
            i++;
            if (i < s.size() && s[i] == '=') { t.type = TNE; i++; }
            else { t.type = TNOT; }
        }
        else if (c == '=') {
            i++; if (i < s.size() && s[i] == '=') i++;
            t.type = TEQ;
        }
        else if (c == '<') {
            i++;
            if (i < s.size() && s[i] == '=') { t.type = TLE; i++; }
            else { t.type = TLT; }
        }
        else if (c == '>') {
            i++;
            if (i < s.size() && s[i] == '=') { t.type = TGE; i++; }
            else { t.type = TGT; }
        }
        else if (c == '&') {
            i++;
            if (i < s.size() && s[i] == '&') i++;
            t.type = TAND;
        }
        else if (c == '|') {
            i++;
            if (i < s.size() && s[i] == '|') i++;
            t.type = TOR;
        }
        else {
            qWarning() << "ExprEval: unexpected char" << c;
            t.type = TERR;
            i++;
        }
        m_tokens.append(t);
    }
    Token endTok; endTok.type = TEND; m_tokens.append(endTok);
}

ExprEval::Token ExprEval::peek()
{
    if (m_pos < m_tokens.size()) return m_tokens[m_pos];
    Token t; t.type = TERR; return t;
}

ExprEval::Token ExprEval::advance()
{
    if (m_pos < m_tokens.size()) return m_tokens[m_pos++];
    Token t; t.type = TERR; return t;
}

qreal ExprEval::parse()       { return parseOr(); }

qreal ExprEval::parseOr()
{
    qreal left = parseAnd();
    while (peek().type == TOR) {
        advance();
        qreal right = parseAnd();
        left = (left > 0.5 || right > 0.5) ? 1.0 : 0.0;
    }
    return left;
}

qreal ExprEval::parseAnd()
{
    qreal left = parseComparison();
    while (peek().type == TAND) {
        advance();
        qreal right = parseComparison();
        left = (left > 0.5 && right > 0.5) ? 1.0 : 0.0;
    }
    return left;
}

qreal ExprEval::parseComparison()
{
    qreal left = parseTerm();
    TokenType op = peek().type;
    if (op == TLT || op == TGT || op == TLE || op == TGE || op == TEQ || op == TNE) {
        advance();
        qreal right = parseTerm();
        switch (op) {
        case TLT: return left <  right ? 1.0 : 0.0;
        case TGT: return left >  right ? 1.0 : 0.0;
        case TLE: return left <= right ? 1.0 : 0.0;
        case TGE: return left >= right ? 1.0 : 0.0;
        case TEQ: return qFabs(left - right) < 1e-9 ? 1.0 : 0.0;
        case TNE: return qFabs(left - right) >= 1e-9 ? 1.0 : 0.0;
        default: break;
        }
    }
    return left;
}

qreal ExprEval::parseTerm()
{
    qreal left = parseFactor();
    while (peek().type == TPLUS || peek().type == TMINUS) {
        Token op = advance();
        qreal right = parseFactor();
        left = (op.type == TPLUS) ? (left + right) : (left - right);
    }
    return left;
}

qreal ExprEval::parseFactor()
{
    qreal left = parseUnary();
    while (peek().type == TSTAR || peek().type == TSLASH) {
        Token op = advance();
        qreal right = parseUnary();
        if (op.type == TSLASH) {
            if (qFabs(right) < 1e-9) { left = 0; qWarning() << "ExprEval: div by zero"; }
            else left /= right;
        } else {
            left *= right;
        }
    }
    return left;
}

qreal ExprEval::parseUnary()
{
    if (peek().type == TMINUS) { advance(); return -parseUnary(); }
    if (peek().type == TNOT)   { advance(); return parseUnary() > 0.5 ? 0.0 : 1.0; }
    return parsePrimary();
}

qreal ExprEval::parsePrimary()
{
    Token t = advance();
    if (t.type == TNUM) return t.numVal;
    if (t.type == TVAL) return m_val;
    if (t.type == TLPAREN) {
        qreal v = parse();
        if (advance().type != TRPAREN)
            qWarning() << "ExprEval: missing ')'";
        return v;
    }
    if (t.type != TEND)
        qWarning() << "ExprEval: unexpected token" << t.type;
    return 0;
}

// ── DataBinding ──────────────────────────────────────────────────

QHash<QString, DataBinding::SetterFunc> &DataBinding::setters()
{
    static QHash<QString, SetterFunc> s;
    return s;
}

void DataBinding::ensureSetters()
{
    auto &s = setters();
    if (!s.isEmpty()) return;

    s["TankWidget.level"]    = [](QWidget *w, qreal v) { static_cast<TankWidget*>(w)->setLevel(qBound(0.0, v, 100.0)); };
    s["TankWidget.alarm"]    = [](QWidget *w, qreal v) { static_cast<TankWidget*>(w)->setAlarm(v > 0.5); };
    s["TankWidget.setpoint"] = [](QWidget *w, qreal v) { static_cast<TankWidget*>(w)->setSetpoint(v); };

    s["PumpWidget.speed"]    = [](QWidget *w, qreal v) { static_cast<PumpWidget*>(w)->setSpeed(qBound(0.0, v, 100.0)); };
    s["PumpWidget.running"]  = [](QWidget *w, qreal v) { static_cast<PumpWidget*>(w)->setRunning(v > 0.5); };

    s["ValveWidget.opening"] = [](QWidget *w, qreal v) { static_cast<ValveWidget*>(w)->setOpening(qBound(0.0, v, 100.0)); };

    s["PipeWidget_H.flowing"] = [](QWidget *w, qreal v) { static_cast<PipeWidget*>(w)->setFlowing(v > 0.5); };
    s["PipeWidget_V.flowing"] = [](QWidget *w, qreal v) { static_cast<PipeWidget*>(w)->setFlowing(v > 0.5); };

    s["ValueDisplay.value"]   = [](QWidget *w, qreal v) { static_cast<ValueDisplay*>(w)->setValue(v); };
}

DataBinding::SetterFunc DataBinding::getSetter(const QString &typeName,
                                               const QString &property)
{
    ensureSetters();
    QString key = typeName + "." + property;
    return setters().value(key);
}

void DataBinding::bindWidget(SimDataManager *sim, QWidget *widget,
                              const QString &typeName,
                              const QVector<WidgetDef::Binding> &bindings)
{
    if (!sim || !widget) return;

    for (const auto &b : bindings) {
        auto setter = getSetter(typeName, b.property);
        if (!setter) {
            qWarning() << "DataBinding: no setter for" << typeName << b.property;
            continue;
        }

        QString tag = b.tag;
        QString transform = b.transform;

        connect(sim, &SimDataManager::valueChanged, widget,
                [widget, setter, tag, transform](const QString &t, double v) {
                    if (t != tag) return;
                    qreal finalVal = transform.isEmpty() ? v
                                    : ExprEval::evaluate(transform, v);
                    setter(widget, finalVal);
                });
    }
}

void DataBinding::applyInitialValues(SimDataManager *sim, QWidget *widget,
                                     const QString &typeName,
                                     const QVector<WidgetDef::Binding> &bindings)
{
    if (!sim || !widget) return;

    for (const auto &b : bindings) {
        auto setter = getSetter(typeName, b.property);
        if (!setter) continue;

        double rawVal = sim->value(b.tag);
        qreal finalVal = b.transform.isEmpty() ? rawVal
                         : ExprEval::evaluate(b.transform, rawVal);
        setter(widget, finalVal);
    }
}
