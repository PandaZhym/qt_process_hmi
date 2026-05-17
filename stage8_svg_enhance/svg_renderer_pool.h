#ifndef SVG_RENDERER_POOL_H
#define SVG_RENDERER_POOL_H

#include <QSvgRenderer>
#include <QHash>
#include <QByteArray>

// 缓存 QSvgRenderer 实例，避免每帧重复解析 SVG
class SvgRendererPool
{
public:
    static SvgRendererPool &instance();
    ~SvgRendererPool();

    QSvgRenderer *renderer(const QString &key);
    void add(const QString &key, const QByteArray &svgData);

private:
    SvgRendererPool() = default;
    QHash<QString, QSvgRenderer *> m_cache;
};

#endif
