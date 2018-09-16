<!--
\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root.
-->
<script lang="ts">
import { Component, Mixins, Prop, Vue, Watch } from 'vue-property-decorator';
import { Line } from 'vue-chartjs';

@Component
export default class Chart extends Mixins(Line) {
  @Prop({ default: [] }) public in!: Array<[string, number]>;
  @Prop({ default: '' }) public titleText!: string;

  private collection?: {
    labels: string[];
    datasets: any[];
  };

  get options() {
    return {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      legend: {
        display: false,
      },
      scales: {
        xAxes: [{
          type: 'time',
          time: {
            unit: 'second',
            // unitStepSize: 1,
            // displayFormats: {
            //   // 'second': 'MMM DD'
            // }
          },
        }],
        yAxes: [{
          ticks: {
            beginAtZero: true,
          },
        }],
      },
      title: {
        display: Boolean(this.titleText),
        text: this.titleText,
      },
    };
  }

  get labels() {
    const a = Array.isArray(this.in) ? this.in : [];
    return a.map((v) => v[0]);
  }
  get values() {
    const a = Array.isArray(this.in) ? this.in : [];
    return a.map((v) => v[1]);
  }

  @Watch('in')
  public onIn() {
    if (this.collection) {
      this.collection.labels = this.labels;
      this.collection.datasets[0].data = this.values;
      this.$data._chart.update();
    }
  }

  public mounted() {
    this.collection = {
      labels: this.labels,
      datasets: [
        {
          backgroundColor: '#f87979',
          data: this.values,
        },
      ],
    };
    this.renderChart(this.collection, this.options);
  }
}
</script>
