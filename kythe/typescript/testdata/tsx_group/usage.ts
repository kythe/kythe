//- @"'./main'" ref/imports MainModule
import {createDiv} from './main';

//- @createDiv ref LocalCreateDiv
//- LocalCreateDiv aliases CreateDiv
createDiv('hello');
