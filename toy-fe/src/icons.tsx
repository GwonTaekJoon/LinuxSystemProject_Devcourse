import React from 'react';

import SVGTOYLogo from './assets/toy-logo.svg';
import SVGRPIIcon from './assets/rpi.svg';

export const TOYLogo = React.memo((): JSX.Element => <SVGTOYLogo />);
export const RPIIcon = React.memo((): JSX.Element => <SVGRPIIcon />);
