/**
 * @file
 * @brief PWMs Devicetree macro public API header file.
 */

/*
 * Copyright (c) 2020, Linaro Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DEVICETREE_PWMS_H_
#define ZEPHYR_INCLUDE_DEVICETREE_PWMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup devicetree-pwms Devicetree PWMs API
 * @ingroup devicetree
 * @{
 */

/**
 * @brief Get pwm controller "name" (label property) at an index
 *
 * It's an error if the pwm controller referenced by the phandle
 * in property "pwms" at index "idx" has no label property.
 *
 * Example devicetree fragment:
 *
 *     pwm1: pwm-controller@... {
 *             label = "PWM_1";
 *     };
 *
 *     pwm2: pwm-controller@... {
 *             label = "PWM_2";
 *     };
 *
 *     n: node {
 *             pwms = <&pwm1 10 20>, <&pwm2 30 40>;
 *     };
 *
 * Example usage:
 *
 *     DT_PWMS_LABEL_BY_IDX(DT_NODELABEL(n), 1) // "PWM_2"
 *
 * @param node_id node identifier
 * @param idx logical index into the property
 * @return the label property for the referenced node at index idx
 * @see DT_PROP_BY_PHANDLE_IDX()
 */
#define DT_PWMS_LABEL_BY_IDX(node_id, idx) \
	DT_PROP_BY_PHANDLE_IDX(node_id, pwms, idx, label)

/**
 * @brief Get pwm controller "name" (label property) by name
 *
 * It's an error if the pwm controller referenced by the phandle
 * in property "pwms" at index "idx" has no label property.
 *
 * Example devicetree fragment:
 *
 *     pwm1: pwm-controller@... {
 *             label = "PWM_1";
 *     };
 *
 *     pwm2: pwm-controller@... {
 *             label = "PWM_2";
 *     };
 *
 *     n: node {
 *             pwms = <&pwm1 10 20>, <&pwm2 30 40>;
 *             pwm-names = "alpha", "beta";
 *     };
 *
 * Example usage:
 *
 *     DT_PWMS_LABEL_BY_NAME(DT_NODELABEL(n), beta) // "PWM_2"
 *
 * @param node_id node identifier
 * @param name lowercase-and-underscores "pwm" name
 * @return the label property for the referenced node by name
 * @see DT_PHANDLE_BY_NAME()
 */
#define DT_PWMS_LABEL_BY_NAME(node_id, name) \
	DT_PROP(DT_PHANDLE_BY_NAME(node_id, pwms, name), label)

/**
 * @brief Equivalent to DT_PWMS_LABEL_BY_IDX(node_id, 0)
 * @param node_id node identifier
 * @return the label property for the named specifier at index 0
 * @see DT_PWMS_LABEL_BY_IDX()
 */
#define DT_PWMS_LABEL(node_id) DT_PWMS_LABEL_BY_IDX(node_id, 0)

/**
 * @brief Get PWM controller "cell" value at an index
 *
 * Example devicetree fragment:
 *
 *     pwm1: pwm-controller@... {
 *             label = "PWM_1";
 *             #pwm-cells = < 2 >;
 *     };
 *
 *     n: node {
 *             pwms = < &pwm1 10 20 >, < &pwm1 30 40 >;
 *     };
 *
 * Bindings fragment for the gpio0 node:
 *
 *     pwm-cells:
 *       - channel
 *       - flags
 *
 * Example usage:
 *
 *     DT_PWMS_CELL_BY_IDX(DT_NODELABEL(n), channel, 0) // 10
 *     DT_PWMS_CELL_BY_IDX(DT_NODELABEL(n), flags, 1) // 40
 *
 * @param node_id node identifier
 * @param cell binding's cell name within the specifier at index "idx"
 * @param idx logical index into the property
 * @return the value of the cell inside the specifier at index "idx"
 * @see DT_PHA_PHANDLE_IDX()
 */
#define DT_PWMS_CELL_BY_IDX(node_id, cell, idx) \
	DT_PHA_BY_IDX(node_id, pwms, idx, cell)

/**
 * @brief Get pwm controller "cell" value by name
 *
 * Example devicetree fragment:
 *
 *     pwm1: pwm-controller@... {
 *             label = "PWM_1";
 *             #pwm-cells = < 2 >;
 *     };
 *
 *     n: node {
 *             pwms = < &pwm1 10 20 >, < &pwm1 30 40 >;
 *             pwm-names = "alpha", "beta";
 *     };
 *
 * Bindings fragment for the pwm1 node:
 *
 *     pwm-cells:
 *       - bus
 *       - bits
 *
 * Example usage:
 *
 *     DT_PWMS_CELL_BY_NAME(DT_NODELABEL(n), alpha, bus) // 10
 *     DT_PWMS_CELL_BY_NAME(DT_NODELABEL(n), beta, bits) // 40
 *
 * @param node_id node identifier
 * @param name lowercase-and-underscores specifier name
 * @param cell binding's cell name within the specifier referenced as "name"
 * @return the value of the cell inside the named specifier
 * @see DT_PHA_PHANDLE_IDX()
 */
#define DT_PWMS_CELL_BY_NAME(node_id, name, cell) \
	DT_PHA_BY_NAME(node_id, pwms, name, cell)

/**
 * @brief Equivalent to DT_PWMS_CELL_BY_IDX(node_id, cell, 0)
 * @param node_id node identifier
 * @param cell binding's cell name within the specifier at index 0
 * @return the value of the cell inside the specifier at index 0
 * @see DT_PWMS_CELL_BY_IDX()
 */
#define DT_PWMS_CELL(node_id, cell) DT_PWMS_CELL_BY_IDX(node_id, cell, 0)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "name" at an index
 * (see @ref DT_PWMS_LABEL_BY_IDX)
 * @param inst instance number
 * @param idx logical index into the property
 * @return the label property for the named specifier at index idx
 */
#define DT_INST_PWMS_LABEL_BY_IDX(inst, idx) \
	DT_PWMS_LABEL_BY_IDX(DT_DRV_INST(inst), idx)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "name" (label property) by name
 * (see @ref DT_PWMS_LABEL_BY_NAME)
 * @param inst instance number
 * @param name lowercase-and-underscores specifier name
 * @return the label property for the named specifier by name
 */
#define DT_INST_PWMS_LABEL_BY_NAME(inst, name) \
	DT_PWMS_LABEL_BY_NAME(DT_DRV_INST(inst), name)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "name"
 * (see @ref DT_PWMS_LABEL_BY_IDX)
 * @param inst instance number
 * @return the label property for the named specifier at index 0
 */
#define DT_INST_PWMS_LABEL(inst) DT_INST_PWMS_LABEL_BY_IDX(inst, 0)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "cell" value at an index
 * @param inst instance number
 * @param cell binding's cell name within the specifier at index "idx"
 * @param idx logical index into the property
 * @return the value of the cell inside the specifier at index "idx"
 */
#define DT_INST_PWMS_CELL_BY_IDX(inst, cell, idx) \
	DT_PWMS_CELL_BY_IDX(DT_DRV_INST(inst), cell, idx)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "cell" value by name
 * (see @ref DT_PWMS_CELL_BY_NAME)
 * @param inst instance number
 * @param name lowercase-and-underscores specifier name
 * @param cell binding's cell name within the specifier referenced as "name"
 * @return the value of the cell inside the named specifier
 */
#define DT_INST_PWMS_CELL_BY_NAME(inst, name, cell) \
	DT_PWMS_CELL_BY_NAME(DT_DRV_INST(inst), name, cell)

/**
 * @brief Get a DT_DRV_COMPAT pwm controller "cell" value at an index 0
 * @param inst instance number
 * @param cell binding's cell name within the specifier at index 0
 * @return the value of the cell inside the specifier at index 0
 */
#define DT_INST_PWMS_CELL(inst, cell) \
	DT_INST_PWMS_CELL_BY_IDX(inst, cell, 0)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif  /* ZEPHYR_INCLUDE_DEVICETREE_PWMS_H_ */
