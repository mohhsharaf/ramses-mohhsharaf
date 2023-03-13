//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "ramses-client.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-text.h"
#include "CLI/CLI.hpp"

int main(int argc, char* argv[])
{
    CLI::App cli;
    ramses::RamsesFrameworkConfig config;
    config.registerOptions(cli);
    CLI11_PARSE(cli, argc, argv);

    printf("Start ramses-shared-lib-check client only\n");
    ramses::RamsesFramework framework{config};
    framework.isConnected();

    ramses::RamsesClient* ramses(framework.createClient("ramses-shared-lib-check"));
    ramses::Scene* scene = ramses->createScene(ramses::sceneId_t(1u));
    scene->isPublished();

    ramses::FontRegistry fontRegistry;

    printf("End ramses-shared-lib-check client only\n");
    return 0;
}
