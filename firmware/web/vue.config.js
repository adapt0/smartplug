// const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const GenerateCppSourcePlugin = require('./webpack-generate-cpp-source-plugin')
const webpack = require('webpack')

module.exports = {
  lintOnSave: false,
  productionSourceMap: false,

  devServer: {
    proxy: (process.env.PROXY) ? {
      '/api/v1': {
        target: `http://${process.env.PROXY}`,
        ws: true,
        changeOrigin: true
      }
    } : undefined
  },

  chainWebpack(config) {
    config
      // Ignore all locale files of moment.js
      // https://github.com/jmblog/how-to-optimize-momentjs-with-webpack
      .plugin('ignore')
        .use(webpack.IgnorePlugin)
        .tap(() => {
          return [ /^\.\/locale$/, /moment$/ ];
        })
        .end()
      // generate cpp source files for inclusion in firmware image
      .plugin('generate-cpp-source')
        .after('compression-webpack-plugin')
        .use(GenerateCppSourcePlugin)
        .end()

      // .plugin('bundle-analyzer')
      //   .use(BundleAnalyzerPlugin)
      //   .tap(() => [{ analyzerMode: 'static' }])
      //  .end()
    ;
  }
}
