#include "jigappinterface.h"

JigAppInterface::JigAppInterface()
{

}

int JigAppInterface::open()
{
    return false;
}

void JigAppInterface::close()
{

}

void JigAppInterface::reset()
{

}

void JigAppInterface::config()
{

}

bool JigAppInterface::isOpen()
{
    return false;
}

int JigAppInterface::getResult()
{
    return 0;
}

qint64 JigAppInterface::write(const QByteArray &data)
{
    (void) data;
    return 0;
}
