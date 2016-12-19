#include <libsnmp.h>
#include <thread>

#include "snmp_pp/snmp_pp.h"
#include "snmp_pp/collect.h"
#include "snmp_pp/notifyqueue.h"

#include "ConsumerTask.h"
#include "RecvSnmp.h"
using namespace Snmp_pp;

static void callback( int reason, Snmp *snmp, Pdu &pdu, SnmpTarget &target, void *cd)
{
    Vb nextVb;
    GenAddress addr;

    target.get_address(addr);
    UdpAddress from(addr);

    cout << "reason: " << reason << endl
        << "msg: " << snmp->error_msg(reason) << endl
        << "from: " << from.get_printable() << endl;

    Oid id;
    pdu.get_notify_id(id);
    cout << "ID:  " << id.get_printable() << endl;
    cout << "Type:" << pdu.get_type() << endl;

    for (int i=0; i<pdu.get_vb_count(); i++)
    {
        pdu.get_vb(nextVb, i);

        cout << "Oid: " << nextVb.get_printable_oid() << endl
            << "Val: " <<  nextVb.get_printable_value() << endl;
    }

    if (pdu.get_type() == sNMP_PDU_INFORM) 
    {
        cout << "pdu type: " << pdu.get_type() << endl;
        cout << "sending response to inform: " << endl;
        nextVb.set_value("This is the response.");
        pdu.set_vb(nextVb, 0);
        snmp->response(pdu, target);
    }
                
    //ProduceItem((uint8_t *)nextVb.get_printable_value, (uint16_t)1024);
    cout << endl;
}

int RecvSnmp(int trap_port = 162)
{
    //----------[ create a SNMP++ session ]-----------------------------------
    int status; 
    Snmp::socket_startup();  // Initialize socket subsystem

    Snmp snmp(status);                // check construction status
    if ( status != SNMP_CLASS_SUCCESS)
    {
        cout << "SNMP++ Session Create Fail, " << snmp.error_msg(status) << "\n";
        return 1;
    }

    OidCollection oidc;
    TargetCollection targetc;

    snmp.notify_set_listen_port(trap_port);
    status = snmp.notify_register(oidc, targetc, callback, NULL);
    if (status != SNMP_CLASS_SUCCESS)
    {
        cout << "Error register for notify (" << status << "): " << snmp.error_msg(status) << endl;
        exit(1);
    }
    else
    {
        cout << "Waiting for traps/informs..." << endl;
    }

    snmp.start_poll_thread(1);
    while(getchar() != 'q');
    snmp.stop_poll_thread();

    Snmp::socket_cleanup();  // Shut down socket subsystem

    return 0;
}

#if 0
int main()
{
    thread  produce(RecvSnmp, 162);
    produce.join();

    return 0;
}
#endif
