CIS_ONENET module usage guide

Brief:          This module is an implementation of onenet.
Usage:          GCC: In your GCC project Makefile, please add the following:
                     include $(SOURCE_DIR)/middleware/third_party/cis_onenet/module.mk
                KEIL: Drag the middleware/third_party/cis_onenet/ciscore and middleware/third_party/cis_onenet/porting folder to your
                      project. Add the following to include paths:
                      middleware/third_party/cis_onenet/ciscore
                      middleware/third_party/cis_onenet/porting
                      middleware/third_party/cis_onenet/ciscore/er-coap-13
                      middleware/third_party/cis_onenet/ciscore/std_object
                IAR: Drag the middleware/third_party/cis_onenet/ciscore and middleware/third_party/cis_onenet/porting folder to your
                      project. Add the following to include paths:
                      middleware/third_party/cis_onenet/ciscore
                      middleware/third_party/cis_onenet/porting
                      middleware/third_party/cis_onenet/ciscore/er-coap-13
                      middleware/third_party/cis_onenet/ciscore/std_object
Dependency:     Please also include LWIP since this module uses it.
Notice:         None.
Relative doc:   Please refer to the open source user guide under the doc folder for more detail.
