/*
 *  Copyright (C) 2019 KeePassXC Team <team@keepassxc.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <stdio.h>

#include "Import.h"

#include "cli/TextStream.h"
#include "cli/Utils.h"
#include "core/Database.h"

Import::Import()
{
    name = QString("import");
    description = QObject::tr("Import the contents of an XML database.");
    positionalArguments.append({QString("xml"), QObject::tr("XML database export."), QString("")});
}

int Import::executeWithDatabase(QSharedPointer<Database> database, QSharedPointer<QCommandLineParser>)
{
    const QStringList args = parser->positionalArguments();
    QString xmlExport = args.at(1);
    TextStream outputTextStream(Utils::STDOUT, QIODevice::WriteOnly);
    TextStream errorTextStream(Utils::STDERR, QIODevice::WriteOnly);

    QString errorMessage;
    if (!database->import(xmlExport, &errorMessage))  {
        errorTextStream << QObject::tr("Unable to import XML database export %1").arg(errorMessage) << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
