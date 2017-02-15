#include <libsnmp.h>
#include <thread>
#include <syslog.h>

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

    syslog(LOG_INFO, "reason: %d, msg: %s, from: %s", reason, snmp->error_msg(reason), from.get_printable());

    Oid id;
    pdu.get_notify_id(id);
    syslog(LOG_INFO, "ID: %s, Type:%d.", id.get_printable(), pdu.get_type());

    for (int i=0; i<pdu.get_vb_count(); i++)
    {
        pdu.get_vb(nextVb, i);

        syslog(LOG_INFO, "Oid: %s, Val: %s.", nextVb.get_printable_oid(),  nextVb.get_printable_value());
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
}

int RecvSnmp(int trap_port = 162)
{
    int status; 
    Snmp::socket_startup();  // Initialize socket subsystem

    Snmp snmp(status);                // check construction status
    if ( status != SNMP_CLASS_SUCCESS)
    {
        syslog(LOG_INFO, "SNMP++ Session Create Fail, %s",  snmp.error_msg(status));
        return 1;
    }

    OidCollection oidc;
    TargetCollection targetc;

    snmp.notify_set_listen_port(trap_port);
    status = snmp.notify_register(oidc, targetc, callback, NULL);
    if (status != SNMP_CLASS_SUCCESS)
    {
        syslog(LOG_INFO, "Error register for notify (%d):%s", status,  snmp.error_msg(status));
        exit(1);
    }
    else
    {
        syslog(LOG_INFO, "Waiting for traps/informs...");
    }

    snmp.start_poll_thread(1);
    while(getchar() != 'q');
    snmp.stop_poll_thread();

    Snmp::socket_cleanup();  // Shut down socket subsystem

    return 0;
}
