#include "kdeworkspace.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>

// Read-only manual smoke check; never sends an activation request.
int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    KdeWorkspace workspace;
    auto report = [&] {
        if (!workspace.state().available) {
            return;
        }
        QTextStream(stdout) << "Ready: " << workspace.state().activities.size()
            << " activities, " << workspace.state().desktops.size() << " desktops.\n";
        application.exit(0);
    };
    QObject::connect(&workspace, &Workspace::stateChanged, &application, report);
    QTimer::singleShot(0, &application, report);
    QTimer::singleShot(6000, &application, [&] {
        QTextStream(stderr) << workspace.state().status << '\n';
        application.exit(1);
    });
    return application.exec();
}
