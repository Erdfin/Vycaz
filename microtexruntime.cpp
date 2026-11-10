#include "microtexruntime.h"

#include "latex.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

bool ensureMicroTeXInitialized()
{
    static const bool initialized = [] {
        const QString appDir = QCoreApplication::applicationDirPath();
        const QStringList candidates{
            appDir + QStringLiteral("/res"),
            appDir + QStringLiteral("/../res"),
            QDir::current().filePath(QStringLiteral("res"))
        };

        for (const QString& candidate : candidates) {
            const QDir dir(candidate);
            if (!QFileInfo::exists(dir.filePath(QStringLiteral(".clatexmath-res_root")))) {
                continue;
            }
            try {
                tex::LaTeX::init(dir.absolutePath().toStdString());
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    }();

    return initialized;
}
