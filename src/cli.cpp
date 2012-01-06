#include "Csl.h"
#include "engine/CslEngine.h"
#if 0
class CslApp: public wxAppConsole
{
    public:
    private:
        CslEngine *m_engine;

        virtual bool OnInit();
        virtual int OnRun();
        virtual int OnExit();

        DECLARE_EVENT_TABLE()
};

DECLARE_APP(CslApp);
IMPLEMENT_APP(CslApp)

bool CslApp::OnInit()
{
    m_engine=new CslEngine;

    return true;
}

int CslApp::OnExit()
{
    if (m_engine)
        delete m_engine;

    return 0;
}
#endif
