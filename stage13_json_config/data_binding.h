#ifndef DATA_BINDING_H
#define DATA_BINDING_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <functional>
#include "config_parser.h"

class SimDataManager;
class QWidget;

// Minimal recursive-descent expression evaluator
// Supports: + - * / < > <= >= == != && || ! val NUM ( expr )
class ExprEval
{
public:
    static qreal evaluate(const QString &expr, qreal val);

private:
    ExprEval(const QString &expr, qreal val);

    enum TokenType { TNUM, TVAL, TPLUS, TMINUS, TSTAR, TSLASH,
                     TLT, TGT, TLE, TGE, TEQ, TNE,
                     TAND, TOR, TNOT, TLPAREN, TRPAREN, TEND, TERR };
    struct Token { TokenType type; qreal numVal = 0; };

    void   tokenize();
    Token  peek();
    Token  advance();
    qreal  parse();
    qreal  parseOr();
    qreal  parseAnd();
    qreal  parseComparison();
    qreal  parseTerm();
    qreal  parseFactor();
    qreal  parseUnary();
    qreal  parsePrimary();

    QString m_expr;
    qreal   m_val = 0;
    QVector<Token> m_tokens;
    int     m_pos = 0;
};

// Wire SimDataManager signals → widget setters per binding definitions
class DataBinding : public QObject
{
    Q_OBJECT
public:
    static void bindWidget(SimDataManager *sim, QWidget *widget,
                           const QString &typeName,
                           const QVector<WidgetDef::Binding> &bindings);

    static void applyInitialValues(SimDataManager *sim, QWidget *widget,
                                   const QString &typeName,
                                   const QVector<WidgetDef::Binding> &bindings);

private:
    using SetterFunc = std::function<void(QWidget *, qreal)>;

    static QHash<QString, SetterFunc> &setters();
    static void ensureSetters();
    static SetterFunc getSetter(const QString &typeName, const QString &property);
};

#endif
