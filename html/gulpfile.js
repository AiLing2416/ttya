const { src, dest, task, series } = require('gulp');
const clean = require('gulp-clean');
const gzip = require('gulp-gzip');
const inlineSource = require('gulp-inline-source');
const rename = require('gulp-rename');
const through2 = require('through2');

const genHeader = (size, buf, len) => {
    const parts = new Array(len * 2 + 4);
    parts[0] = 'unsigned char index_html[] = {\n  ';

    let idx = 0;
    let p = 1;
    for (const value of buf) {
        idx++;
        const current = value < 0 ? value + 256 : value;

        parts[p++] = '0x' + (current >>> 4).toString(16) + (current & 0xf).toString(16);

        if (idx === len) {
            parts[p++] = '\n';
        } else {
            parts[p++] = idx % 12 === 0 ? ',\n  ' : ', ';
        }
    }

    parts[p++] = '};\n';
    parts[p++] = `unsigned int index_html_len = ${len};\n`;
    parts[p++] = `unsigned int index_html_size = ${size};\n`;
    return parts.join('');
};
let fileSize = 0;

task('clean', () => {
    return src('dist', { read: false, allowEmpty: true }).pipe(clean());
});

task('inline', () => {
    const options = {
        compress: false,
    };

    return src('dist/index.html').pipe(inlineSource(options)).pipe(rename('inline.html')).pipe(dest('dist/'));
});

task(
    'default',
    series('inline', () => {
        return src('dist/inline.html')
            .pipe(
                through2.obj((file, enc, cb) => {
                    fileSize = file.contents.length;
                    return cb(null, file);
                })
            )
            .pipe(gzip())
            .pipe(
                through2.obj((file, enc, cb) => {
                    const buf = file.contents;
                    file.contents = Buffer.from(genHeader(fileSize, buf, buf.length));
                    return cb(null, file);
                })
            )
            .pipe(rename('html.h'))
            .pipe(dest('../src/'));
    })
);
