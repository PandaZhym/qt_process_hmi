#include "svg_renderer_pool.h"

SvgRendererPool &SvgRendererPool::instance()
{
    static SvgRendererPool pool;
    return pool;
}

SvgRendererPool::~SvgRendererPool()
{
    qDeleteAll(m_cache);
    m_cache.clear();
}

QSvgRenderer *SvgRendererPool::renderer(const QString &key)
{
    return m_cache.value(key, nullptr);
}

void SvgRendererPool::add(const QString &key, const QByteArray &svgData)
{
    if (m_cache.contains(key)) return;
    m_cache[key] = new QSvgRenderer(svgData);
}
