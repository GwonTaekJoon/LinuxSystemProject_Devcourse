#!/usr/bin/env node
const chalk = require('chalk');
const airbnbLogoText = require('./airbnbLogoText');
const rausch = chalk.rgb(255, 90, 95);

console.log(rausch(airbnbLogoText));