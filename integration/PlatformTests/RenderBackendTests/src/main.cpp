//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "RendererTestUtils.h"
#include "Utils/ThreadLocalLog.h"
#include "CLI/CLI.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleMock(&argc, argv);

    ramses::RamsesFrameworkConfig frameworkConfig;
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;
    CLI::App cli;
    frameworkConfig.registerOptions(cli);
    rendererConfig.registerOptions(cli);
    displayConfig.registerOptions(cli);

    CLI11_PARSE(cli, argc, argv);
    RendererTestUtils::SetDefaultConfigForAllTests(rendererConfig, displayConfig);

    // set log prefix for all tests
    ramses_internal::ThreadLocalLog::SetPrefix(1);

    return RUN_ALL_TESTS();
}
