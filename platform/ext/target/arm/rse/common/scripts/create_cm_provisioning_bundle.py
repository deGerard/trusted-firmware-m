#!/usr/bin/env python3
#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

import argparse
import sys
import os

import arg_utils

import logging
logger = logging.getLogger("TF-M")

sys.path.append(os.path.join(sys.path[0], 'modules'))

import otp_config as oc
from otp_config import OTP_config

import provisioning_message_config as pmc
from provisioning_message_config import Provisioning_message_config

import provisioning_config as pc
from provisioning_config import Provisioning_config


def add_arguments(parser : argparse.ArgumentParser,
                  prefix : str = "",
                  required : bool = True,
                  ) -> None:
    oc.add_arguments(parser, prefix, required)
    pc.add_arguments(parser, prefix, required, regions=["cm"])
    pmc.add_arguments(parser, prefix, required,
                      message_type="RSE_PROVISIONING_MESSAGE_TYPE_BLOB")

    arg_utils.add_prefixed_argument(parser, "provisioning_code_elf", prefix, help="provisioning code image elf file",
                                            type=arg_utils.arg_type_elf_section(["CODE", "DATA"]), required=True)

def parse_args(args : argparse.Namespace,
               prefix : str = "",
               ) -> dict:
    out = {}
    out |= dict(zip(["code", "data"], arg_utils.get_arg(args, "provisioning_code_elf", prefix)))

    out |= oc.parse_args(args, prefix=prefix)
    out |= pc.parse_args(args, prefix=prefix, otp_config = out["otp_config"])
    out |= pmc.parse_args(args, prefix=prefix)

    return out


script_description = """
This script takes as various config files, and produces from them and input
arguments corresponding to the fields of the CM provisioning bundle, and
produces a signed CM provisioning bundle which can be input into the RSE for
provisioning CM data
"""
if __name__ == "__main__":
    from provisioning_message_config import create_blob_message

    parser = argparse.ArgumentParser(allow_abbrev=False,
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     description=script_description)

    add_arguments(parser)

    parser.add_argument("--bundle_output_file",    help="bundle output file", required=True)
    parser.add_argument("--log_level", help="log level", required=False, default="ERROR", choices=logging._levelToName.values())

    args = parser.parse_args()
    logger.setLevel(args.log_level)

    kwargs = parse_args(args)

    kwargs['otp_config'].set_cm_offsets_automatically()
    kwargs['provisioning_config'].set_area_infos_from_otp_config(**kwargs)

    logging.debug(kwargs['provisioning_config'].cm_layout)

    blob_type = kwargs['provisioning_message_config'].RSE_PROVISIONING_BLOB_TYPE_SINGLE_LCS_PROVISIONING

    with open(args.bundle_output_file, "wb") as f:
        message = create_blob_message(blob_type=blob_type, **kwargs,
                                      secret_values = kwargs['provisioning_config'].cm_layout.to_bytes())
        f.write(message)
