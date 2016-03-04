/*eslint-disable */
var path = require('path');
var HtmlWebpackPlugin = require('html-webpack-plugin')
var fs = require('fs');
var webpack = require('webpack');

module.exports = {
  entry: './srcweb/main.js',
  output: {
      path: path.join(__dirname, 'data'),
      filename: 'bundle.js'
  },
  module: {
      loaders: [
          {
            test: /\.js?$/,
            exclude: /node_modules/,
            loader: 'babel-loader'
          }
      ]
  },
  plugins: [
    new HtmlWebpackPlugin({
      title: 'Display Controll',
      inject: true,
      template: './srcweb/index.html'
    }),
    new webpack.optimize.UglifyJsPlugin({minimize: true})
  ]
};
