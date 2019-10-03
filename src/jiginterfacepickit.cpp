#include "jiginterfacepickit.h"

JigInterfacePickit::JigInterfacePickit()
{
    ipe = new Pickit(this);
}

int JigInterfacePickit::open()
{
    return true;
}

void JigInterfacePickit::close()
{
    ipe->close();
}

void JigInterfacePickit::config() {}
