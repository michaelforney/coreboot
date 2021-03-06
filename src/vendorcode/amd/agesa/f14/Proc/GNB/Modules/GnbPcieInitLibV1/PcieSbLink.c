/* $NoKeywords:$ */
/**
 * @file
 *
 * GNB-SB link procedure
 *
 *
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:     AGESA
 * @e sub-project: GNB
 * @e \$Revision: 38931 $   @e \$Date: 2010-10-01 15:50:05 -0700 (Fri, 01 Oct 2010) $
 *
 */
/*
 *****************************************************************************
 *
 * Copyright (c) 2011, Advanced Micro Devices, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the names of 
 *       its contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***************************************************************************
 *
 */

/*----------------------------------------------------------------------------------------
 *                             M O D U L E S    U S E D
 *----------------------------------------------------------------------------------------
 */
#include  "AGESA.h"
#include  "Ids.h"
#include  "amdlib.h"
#include  "Gnb.h"
#include  "GnbPcie.h"
#include  GNB_MODULE_DEFINITIONS (GnbCommonLib)
#include  GNB_MODULE_DEFINITIONS (GnbPcieInitLibV1)
#include  "GnbRegistersON.h"
#include  "Filecode.h"
#define FILECODE PROC_GNB_MODULES_GNBPCIEINITLIBV1_PCIESBLINK_FILECODE
/*----------------------------------------------------------------------------------------
 *                   D E F I N I T I O N S    A N D    M A C R O S
 *----------------------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------------------
 *                  T Y P E D E F S     A N D     S T R U C T U  R E S
 *----------------------------------------------------------------------------------------
 */


/*----------------------------------------------------------------------------------------
 *           P R O T O T Y P E S     O F     L O C A L     F U  N C T I O N S
 *----------------------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------------------*/
/**
 * Enable/Disable ASPM on GNB-SB link
 *
 *
 *
 * @param[in]  Engine          Pointer to engine config descriptor
 * @param[in]  Pcie            Pointer to global PCIe configuration
 *
 */

AGESA_STATUS
PcieSbLinkAspmControl (
  IN      PCIe_ENGINE_CONFIG    *Engine,
  IN      PCIe_PLATFORM_CONFIG  *Pcie
  )
{
  AGESA_STATUS  Status;
  UINT8         NbAspm;

  Status = PcieSbInitAspm (Engine->Type.Port.PortData.LinkAspm, GnbLibGetHeader (Pcie));
  if (Status != AGESA_SUCCESS) {
    return AGESA_UNSUPPORTED;
  }

  NbAspm = Engine->Type.Port.PortData.LinkAspm;

  PcieNbAspmEnable (Engine->Type.Port.Address, NbAspm, GnbLibGetHeader (Pcie));
  return  AGESA_SUCCESS;
}

/*----------------------------------------------------------------------------------------*/
/**
 * Init SB ASPM.
 * Enable ASPM states on SB
 *
 *
 * @param[in] Aspm       ASPM bitmap.
 * @param[in] StdHeader  Standard configuration header
 */
/*----------------------------------------------------------------------------------------*/

AGESA_STATUS
PcieSbInitAspm (
  IN      PCIE_ASPM_TYPE       Aspm,
  IN      AMD_CONFIG_PARAMS   *StdHeader
  )
{
  AGESA_STATUS  Status;
  UINT16        AlinkPort;

  Status = PcieSbAgetAlinkIoAddress (&AlinkPort, StdHeader);
  if (Status != AGESA_SUCCESS) {
    return Status;
  }
  GnbLibIoRMW (AlinkPort, AccessS3SaveWidth32, 0x0, 0x40000038, StdHeader);
  GnbLibIoRMW (AlinkPort + 4, AccessS3SaveWidth32, 0x0, 0xA0, StdHeader);
  GnbLibIoRMW (AlinkPort, AccessS3SaveWidth32, 0x0, 0x4000003c, StdHeader);
  GnbLibIoRMW (AlinkPort + 4, AccessS3SaveWidth32, 0xffff00ff, 0x6900, StdHeader);
  GnbLibIoRMW (AlinkPort, AccessS3SaveWidth32, 0x0, 0x80000068, StdHeader);
  GnbLibIoRMW (AlinkPort + 4, AccessS3SaveWidth32, 0xfffffffc, Aspm, StdHeader);
  return AGESA_SUCCESS;
}

/*----------------------------------------------------------------------------------------*/
/**
 * Get Alink config address
 *
 *
 */
/*----------------------------------------------------------------------------------------*/

AGESA_STATUS
PcieSbAgetAlinkIoAddress (
     OUT   UINT16               *AlinkPort,
  IN       AMD_CONFIG_PARAMS    *StdHeader
  )
{
  UINT8 AlinkPortIndex;
  AlinkPortIndex = 0xE0;
  GnbLibIoWrite (0xCD6, AccessWidth8, &AlinkPortIndex, StdHeader);
  GnbLibIoRead (0xCD7, AccessWidth8, AlinkPort, StdHeader);
  AlinkPortIndex = 0xE1;
  GnbLibIoWrite (0xCD6, AccessWidth8, &AlinkPortIndex, StdHeader);
  GnbLibIoRead (0xCD7, AccessWidth8, (VOID*) ((UINT8*) AlinkPort + 1), StdHeader);
  if (&AlinkPort == 0) {
    return AGESA_UNSUPPORTED;
  }
  return  AGESA_SUCCESS;
}

/*----------------------------------------------------------------------------------------*/
/**
 * Set  ASMP State on PCIe device function
 *
 *
 *
 * @param[in] Function    PCI address of function.
 * @param[in] Aspm        ASPM bitmap.
 * @param[in] StdHeader   Standard configuration header
 *
 */
 /*----------------------------------------------------------------------------------------*/

VOID
PcieNbAspmEnable (
  IN      PCI_ADDR              Function,
  IN      PCIE_ASPM_TYPE        Aspm,
  IN      AMD_CONFIG_PARAMS     *StdHeader
  )
{
  UINT8       PcieCapPtr;
  PcieCapPtr = GnbLibFindPciCapability (Function.AddressValue, PCIE_CAP_ID, StdHeader);
  if (PcieCapPtr != 0) {
    GnbLibPciRMW (
      Function.AddressValue | (PcieCapPtr + PCIE_LINK_CTRL_REGISTER) ,
      AccessS3SaveWidth8,
      ~(BIT0 | BIT1),
      Aspm,
      StdHeader
      );
  }
}

/*----------------------------------------------------------------------------------------*/
/**
 * Enable VC on GNB-SB link
 *
 *
 *
 * @param[in]  Engine          Pointer to engine config descriptor
 * @param[in]  Pcie            Pointer to global PCIe configuration
 *
 */

VOID
PcieSbLinkVcEnable (
  IN      PCIe_ENGINE_CONFIG    *Engine,
  IN      PCIe_PLATFORM_CONFIG  *Pcie
  )
{

}

