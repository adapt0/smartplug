<script>
import { Line } from 'vue-chartjs'

export default {
  extends: Line,
  props: ['in', 'titleText'],
  data () {
    return {
      options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        legend: {
          display: false
        },
        scales: {
          xAxes: [{
            type: 'time',
            time: {
              unit: 'second'
              // unitStepSize: 1,
              // displayFormats: {
              //   // 'second': 'MMM DD'
              // }
            }
          }],
          yAxes: [{
            ticks: {
              beginAtZero: true
            }
          }]
        },
        title: {
          display: Boolean(this.titleText),
          text: this.titleText
        }
      }
    }
  },
  computed: {
    labels () {
      const a = Array.isArray(this.in) ? this.in : []
      return a.map((v) => v[0])
    },
    values () {
      const a = Array.isArray(this.in) ? this.in : []
      return a.map((v) => v[1])
    }
  },
  watch: {
    in () {
      this.collection.labels = this.labels
      this.collection.datasets[0].data = this.values
      this.$data._chart.update()
    }
  },
  mounted () {
    this.collection = {
      labels: this.labels,
      datasets: [
        {
          backgroundColor: '#f87979',
          data: this.values
        }
      ]
    }
    this.renderChart(this.collection, this.options)
  }
}
</script>
