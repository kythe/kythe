// Syntax 1: exporting a value from another module.
export {value} from './module';

// Syntax 2: a bare "export" statement.
const local = 3;
export {local};
export {local as aliasedLocal};
