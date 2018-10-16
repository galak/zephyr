#define DT_APDS9960(bus) \
	/ {						\
		aliases {				\
			apds9960-0 = &apds9960;		\
		};					\
	};						\
							\
	&bus {						\
		apds9960: apds9960@29 {			\
			compatible = "avago,apds9960";	\
			reg = <0x29>;			\
			label = "APDS9960";		\
		};					\
	};
